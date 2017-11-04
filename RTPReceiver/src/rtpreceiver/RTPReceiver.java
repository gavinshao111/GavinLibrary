package rtpreceiver;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.ByteBuffer;

/**
 * Created by gavin on 11/4/16.
 */
public class RTPReceiver {

    private static final short MinRTPHeaderLength = 12;
    private static final short H264TypeCode = 96;
    private static final byte StartOrderCode = 2;
    private static final byte EndOrderCode = 1;
    private static final byte MidOrderCode = 0;
    private static final byte[] NaluStartCode = new byte[]{0, 0, 0, 1};
    public static final int RTP_SIZE = 10000;

    public enum RtpType {
        STAP_A, FU_START, FU_NONSTART, OTHER;
    }
    
    // debug
    //public static PrintWriter s_logPw;

    private static boolean DEBUG;
    private final int[] TypeArray;

    private byte NaluTypeCode;
    private RtpType rtpType;

    private ByteBuffer m_rtpBuf;

    private byte[] m_rtpPacketBuf;
    
    private DatagramSocket m_udpSkt;
    private DatagramPacket m_rcvPacket;
    
    private InputStream m_tcpIs;

    private boolean m_rtpOverTcp;

    private boolean M;  // for video, it is end flag of a frame
    private short PT;           // payload type
    private int seq;
    private long timestamp;
    private long SSRC;
    
    private int m_naluSizeTmp;

    public RTPReceiver() {
        TypeArray = new int[32];
        m_rtpPacketBuf = new byte[RTP_SIZE];
        DEBUG = false;
        m_rcvPacket = null;
        m_tcpIs = null;
        m_naluSizeTmp = 0;
    }

    // DatagramSocket 由外部管理
    public void setupWithUdp(DatagramSocket udpSkt) throws SocketException {
        m_udpSkt = udpSkt;
        m_udpSkt.setSoTimeout(1000 * 10);
        
        m_rcvPacket = new DatagramPacket(m_rtpPacketBuf, m_rtpPacketBuf.length);
        m_rtpOverTcp = false;
    }

    public void setupWithTcp(Socket skt) throws IOException {
        skt.setSoTimeout(1000 * 10);
        m_tcpIs = skt.getInputStream();
        m_rtpOverTcp = true;
    }

    public void close() throws IOException {
        if (m_rtpOverTcp && m_tcpIs != null) {
            m_tcpIs.close();
        }
    }

    public ByteBuffer GetNextRTP() throws IOException {
        ReadNextRTPPacket();
        return parseRtpAndGetNaluWithStartCode();
    }

    public int getSeq() {
        return seq;
    }

    public String HdrToString() {
        return "SingleRTPPacket{"
                + "M=" + M
                + ", PT=" + PT
                + ", seq=" + seq
                + ", timestamp=" + timestamp
                + ", SSRC=" + SSRC
                + //", RTPByteBuf=" + RTPByteBuf +
                '}';
    }

    /**
     * this method will be called cyclically for(;;) ReadNextRTPPacket();
     *
     * RTP format: | magic number(1 byte: '$') | channel number(1 byte, 0 for
     * RTP, 1 for RTCP) | embedded data length(2 bytes) | data |
     *
     * after this, RTPByteBuf is fill with RTP data without header, pos is at
     * start of RTP Payload.
     *
     * return 1 when read time out. return 2 when read to end.
     */
    private void ReadNextRTPPacket() throws IOException {

        int count = 0;
        short d;
        int rc;
        int rtpPacketSize;
        for (;;) {
            if (m_rtpOverTcp) {
                int c = m_tcpIs.read();
                if (c != (int) '$') {
                    if (c == -1) {
                        break;
                    }
                    count++;
                    if (count > 100000) {
                        System.out.printf("[WARN] ReadNextRTPPacket: %d bytes ignored for waiting '$'.\n", count);
                        count = 0;
                    }
                    continue;
                }
                short channelNumber = (short) m_tcpIs.read();
                rtpPacketSize = m_tcpIs.read() << 8 | m_tcpIs.read();
                if (rtpPacketSize > RTP_SIZE)
                    throw new RuntimeException("rtpPacketSize: " + rtpPacketSize + " is larger then " + RTP_SIZE);
//                s_logPw.println("rtp size: " + rtpPacketSize);
                //System.out.println("rtp size: " + rtpPacketSize);
                int curBytesRead = 0;
                while (curBytesRead < rtpPacketSize) {
                    curBytesRead += m_tcpIs.read(m_rtpPacketBuf, curBytesRead, rtpPacketSize - curBytesRead);
                }                
                if (channelNumber == 1) {
                    System.out.println("[INFO] ReadNextRTPPacket: get rtcp data");
                    continue;
                } else if (channelNumber != 0) {
                    System.out.println("[WARN] ReadNextRTPPacket: Illegal channelNumber: " + channelNumber);
                    continue;
                }
                m_rtpBuf = ByteBuffer.wrap(m_rtpPacketBuf, 0, rtpPacketSize);
            } else {
                m_udpSkt.receive(m_rcvPacket);
                m_rtpBuf = ByteBuffer.wrap(m_rcvPacket.getData(), m_rcvPacket.getOffset(), m_rcvPacket.getLength());
            }
            
            if (m_rtpBuf.remaining() < MinRTPHeaderLength) {
                continue;
            }
            rc = ParseRTPAndGetRTPPayload();
            if (0 != rc) {
                System.out.printf("[WARN] ReadNextRTPPacket.ParseRTPAndGetRTPPayload fail, rc = %d\n\n", rc);
                continue;   // we will ingore it, just get next packet.
            }

            if (H264TypeCode != PT) {
                System.out.println("[WARN] ReadNextRTPPacket: not a H264 bit stream, RTP payload type code: " + PT);
                continue;   // we will ignore it, just get next packet.
            }

            break;
        }
    }

    private ByteBuffer parseRtpAndGetNaluWithStartCode() {
        if (-1 == processSpecialHeader()) {
            return null;
        }
        ByteBuffer naluInCurrentPacket = ByteBuffer.allocate(RTP_SIZE);
        int frameSize;

        for (;;) {
            frameSize = nextEnclosedFrameSize();
            if (0 == frameSize) {
                break;
            }

            if (m_rtpBuf.remaining() < frameSize) {
                return null;
            }
            if (rtpType != RtpType.FU_NONSTART) {
                naluInCurrentPacket.put(NaluStartCode);
                // debug
                //if (m_naluSizeTmp > 0)  // 写入上一个nalu的长度
                    //s_logPw.println("nalu size: " + m_naluSizeTmp);
                m_naluSizeTmp = 0;
            }
            m_naluSizeTmp += frameSize;
            //s_logPw.println("m_naluSizeTmp: " + m_naluSizeTmp);
            naluInCurrentPacket.put(m_rtpBuf.array(), m_rtpBuf.position(), frameSize);
            m_rtpBuf.position(m_rtpBuf.position() + frameSize);

        }
        naluInCurrentPacket.flip();
//        s_logPw.println("Nalu size With StartCode in current rtp: " + naluInCurrentPacket.remaining());
        return naluInCurrentPacket;
    }

    /**
     * |V(2)|P(1)|X(1)|cc(4)|M(1)|PT(7)|seq(16)|timestamp(32)|SSRC(32)|
     * |<-- header 12 bytes                                        -->| | CSRC (
     * cc * 4 * bytes) ... |(2bytes)|NumOfRemExtHdr(2bytes)| remain extension
     * header ... |<-- ExtHdr, length = (1+NumOfRemExtHdr)*4
     */
    private int ParseRTPAndGetRTPPayload() {
        if (null == m_rtpBuf || !m_rtpBuf.hasRemaining()) {
            return -1;
        }

        byte ByteTmp = m_rtpBuf.get();

        byte version = (byte) ((ByteTmp >> 6) & 0x03);
        if (2 != version) {   //RTP version should be 2
            System.out.printf("[WARN] RTP version check fail: %d\n", version);
            return -2;
        }

        boolean P = (ByteTmp & 0x20) > 0;   // flag of padding
        boolean X = (ByteTmp & 0x10) > 0;   // flag of extension header
        short cc = (short) (ByteTmp & 0xf);                 // is number of CSRC

        ByteTmp = m_rtpBuf.get();
        M = (ByteTmp & 0x80) > 0;     // for video, it is end flag of a frame
        PT = (short) (ByteTmp & 0x7f);                  // payload type
        seq = m_rtpBuf.getShort() & 0xffff;
        timestamp = m_rtpBuf.getInt() & 0xffffffffL;
        SSRC = m_rtpBuf.getInt() & 0xffffffffL;

        //if(true)
        if (DEBUG) {
            System.out.println("[DEBUG] P: " + P + " X: " + X + " cc: " + cc + HdrToString());
        }

        if (m_rtpBuf.remaining() < cc * 4) {
            return -3;
        }
        if (cc > 0) {
            m_rtpBuf.position(m_rtpBuf.position() + cc * 4);    //skip CSRC
        }
        if (X) {
            m_rtpBuf.position(m_rtpBuf.position() + 2);
            int NumOfRemExtHdr = m_rtpBuf.getShort();
            if (m_rtpBuf.remaining() < NumOfRemExtHdr * 4) {
                return -4;
            }
            m_rtpBuf.position(m_rtpBuf.position() + NumOfRemExtHdr * 4);    //skip RemExtHdr
        }

        if (P) {
            short LenOfPadding = m_rtpBuf.get(m_rtpBuf.limit() - 1);
            if (m_rtpBuf.remaining() < LenOfPadding) {
                return -5;
            }
            m_rtpBuf.limit(m_rtpBuf.limit() - LenOfPadding);
        }

        return 0;
    }

    /**
     * analyze RTP PL data get Nalu type, OrderCode, and write data to
     * NaluPLByteBuf
     *
     * EasyDarwin server's most NALU types are FU-A(code = 28) and STAP-A(24),
     * so we only analyze these 2 types. at this time, RTPByteBuf should be
     * like: for FU-A | Indicator(1byte) | FU-A header(1byte) | Nalu data |
     * Indicator: | F(1bit) | NRI(2bits) | Nal Packet Type(5bits) | FU-A header:
     * | S(1) | E(1) | not care(1) | Nal PL Type(5) |
     *
     * for STAP-A | Indicator(1byte) | length(2bytes) | header(1byte) | NALU
     * data | ... | length(2bytes) | header(1byte) | NALU data | ... | Indicator
     * is same as FU-A.
     *
     * return value: 1, Nalu get end. 0, not end and ok. <0, error
     *
     * since ReadNextRTPPacket will flip RTPByteBuf after read, so this func
     * will follow this rule, flip NaluPLByteBuf after copy data from
     * RTPByteBuf. so after call this func, you can read data from NaluPLByteBuf
     * without other operation.
     *
     */
    /**
     * 去掉RTP包头部之后，在去掉特殊头部。即指针指向特殊头部之后。若Type为28且为start，
     * 则将组装后的Nalu的头部写入RTPByteBuf，并把指针指向这个头部。 return val: 0: it is start of a
     * nalu. 1: not a start nal. -1: format error.
     */
    private int processSpecialHeader() {
        int packetSize = m_rtpBuf.remaining();
        int RTPPLPos = m_rtpBuf.position();
        if (packetSize < 1) {
            return -1;
        }
        NaluTypeCode = (byte) (m_rtpBuf.get(RTPPLPos) & 0x1F);

        if (DEBUG) {
            System.out.println("get Indicator: " + Integer.toHexString(m_rtpBuf.get(RTPPLPos) & 0xFF));
        }

        if (NaluTypeCode < 32) {
            TypeArray[NaluTypeCode]++;
        } else {
            TypeArray[32]++;
        }

//        if (NaluTypeCode != 28)
        //System.out.println("[processSpecialHeader] Type: " + NaluTypeCode);
        switch (NaluTypeCode) {
            case 24: { // STAP-A
                rtpType = RtpType.STAP_A;
                // numBytesToSkip = 1; // discard the type byte
                m_rtpBuf.position(RTPPLPos + 1);
                break;
            }
            case 25:
            case 26:
            case 27: { // STAP-B, MTAP16, or MTAP24
                // numBytesToSkip = 3; // discard the type byte, and the initial DON
                rtpType = RtpType.OTHER;
                m_rtpBuf.position(RTPPLPos + 3);
                break;
            }
            case 28:
            case 29: { // // FU-A or FU-B
                // For these NALUs, the first two bytes are the FU indicator and the FU header.
                // If the start bit is set, we reconstruct the original NAL header into byte 1:
                if (packetSize < 2) {
                    return -1;
                }
                byte OrderCode = (byte) (m_rtpBuf.get(RTPPLPos + 1) >> 6 & 3);

                if (StartOrderCode == OrderCode) {
                    rtpType = RtpType.FU_START;
                    m_rtpBuf.array()[RTPPLPos + 1] = (byte) ((m_rtpBuf.get(RTPPLPos) & 0xe0)
                            | (m_rtpBuf.get(RTPPLPos + 1) & 0x1f));

                    // numBytesToSkip = 1;
                    m_rtpBuf.position(RTPPLPos + 1);
                } else {
                    // The start bit is not set, so we skip both the FU indicator and header:
                    // fCurrentPacketBeginsFrame = False;
                    // numBytesToSkip = 2;
                    rtpType = RtpType.FU_NONSTART;
                    m_rtpBuf.position(RTPPLPos + 2);
                }
                break;
            }
            default: {
                // This packet contains one complete NAL unit:
                // numBytesToSkip = 0;
                rtpType = RtpType.OTHER;
                break;
            }
        }

        return 0;

    }

    /**
     * 指针略过需要忽略的部分，比如type为24的类型，需要略过2字节的长度。并返回需要读取的长度。
     */
    private int nextEnclosedFrameSize(/*unsigned char*& framePtr*/) {
        int resultNALUSize = 0; // if an error occurs

        switch (NaluTypeCode) {
            case 24:
            case 25: { // STAP-A or STAP-B
                // The first two bytes are NALU size:
                if (m_rtpBuf.remaining() < 2) {
                    break;
                }

                // resultNALUSize = (framePtr[0] << 8) | framePtr[1];
                // framePtr += 2;
                resultNALUSize = m_rtpBuf.getShort() & 0xffff;

                break;
            }
            case 26: { // MTAP16
                // The first two bytes are NALU size.  The next three are the DOND and TS offset:
                if (m_rtpBuf.remaining() < 5) {
                    break;
                }
                // resultNALUSize = (framePtr[0] << 8) | framePtr[1];
                // framePtr += 5;
                resultNALUSize = m_rtpBuf.getShort() & 0xffff;
                m_rtpBuf.position(m_rtpBuf.position() + 3);

                break;
            }
            case 27: { // MTAP24
                // The first two bytes are NALU size.  The next four are the DOND and TS offset:
                if (m_rtpBuf.remaining() < 6) {
                    break;
                }
//                resultNALUSize = (framePtr[0] << 8) | framePtr[1];
//                framePtr += 6;
                resultNALUSize = m_rtpBuf.getShort() & 0xffff;
                m_rtpBuf.position(m_rtpBuf.position() + 4);

                break;
            }
            default: {
                // Common case: We use the entire packet data:
                return m_rtpBuf.remaining();
            }
        }

        if (resultNALUSize > m_rtpBuf.remaining()) {
            System.out.println("[WARN] Read frameSize error. remaining is less then frameSize. Remaining will be copied to buf."
                    + "\nframeSize: " + resultNALUSize
                    + " Remaining: " + m_rtpBuf.remaining()
                    + " Type: " + NaluTypeCode
            );

            return m_rtpBuf.remaining();
        }
        if (m_rtpBuf.limit() - 12 < m_rtpBuf.remaining()) {
            System.out.println("[WARN] RTPByteBuf remaining error. Nothing to copy."
                    + "\nframeSize: " + resultNALUSize
                    + " Remaining: " + m_rtpBuf.remaining()
                    + " Type: " + NaluTypeCode
            );
            return 0;

        }

        return resultNALUSize;
    }

    public void PrintTypeCount() {
        byte i = 0;
        for (; i < TypeArray.length; i++) {
            if (TypeArray[i] > 0) {
                System.out.println(i + " " + TypeArray[i]);
            }
        }
    }

}
