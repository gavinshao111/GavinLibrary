/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package rtsppushclient;

import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Calendar;
import java.util.logging.Logger;

/**
 *
 * @author 10256
 */
public class RTPStream {

    /**
     *
     */
    public static enum NaluType {
        // 单一 NAL 单元模式，一个Nal单元组成一个RTP包
        SingleNalu((byte) 1),
        // 单一时间的组合包，多个Nal单元组成一个RTP包
        STAP_A((byte) 24),
        // 分片的单元，一个Nal单元拆分为多个RTP包
        FU_A((byte) 28);

        private final byte value;

        private NaluType(byte value) {
            this.value = value;
        }

        public byte value() {
            return value;
        }
    }

    // 通过 ping -f -l 1472 ip 测试得到
    public static final int MTU_SIZE = 1472;
    public static final int RTP_HEADER_SIZE = 12;
    private static final int MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP = MTU_SIZE - RTP_HEADER_SIZE - 2;

    private static final byte StartOrderCodeForFUA = (byte) (1 << 7);
    private static final byte MiddleOrderCodeForFUA = 0;
    private static final byte EndOrderCodeForFUA = 1 << 6;

    private String m_StreamServerIp;
    private int m_StreamServerPort;

    private DatagramSocket m_udpSkt;
    private InetAddress m_svrAddr;

    private OutputStream m_tcpOs;

    private ByteBuffer m_rtpPayload;
    private ByteBuffer m_rtpHdr;
    private ByteBuffer m_sendBuf;
    private int m_rtpSeq;
    private int NaluTypeArray[];
    private int m_rtpCount;
    private int m_lastSeq;
    private boolean m_rtpOverTcp;

    public RTPStream() {
//        m_rtpPacketBuf = new byte[MTU_SIZE];
//        m_rtpHdr = ByteBuffer.wrap(m_rtpPacketBuf, 0, RTP_HEADER_SIZE);
        m_rtpHdr = ByteBuffer.allocate(RTP_HEADER_SIZE);
//        m_rtpPayload = ByteBuffer.wrap(m_rtpPacketBuf, RTP_HEADER_SIZE, MTU_SIZE - RTP_HEADER_SIZE);
        m_rtpPayload = ByteBuffer.allocate(MTU_SIZE - RTP_HEADER_SIZE);

        /**
         * if rtp over tcp, format:
         *
         * | magic number(1 byte: '$') | channel number(1 byte, 0 for RTP, 1 for
         * RTCP) | embedded data length(2 bytes) |
         */
        m_sendBuf = ByteBuffer.allocate(MTU_SIZE + 4);
        NaluTypeArray = new int[32];
        m_rtpOverTcp = false;
    }

    public void setupWithUdp(String StreamServerIp, int StreamServerPort) throws SocketException, UnknownHostException {
        reset();
        m_StreamServerIp = StreamServerIp;
        m_StreamServerPort = StreamServerPort;
        m_udpSkt = new DatagramSocket();
        m_svrAddr = InetAddress.getByName(m_StreamServerIp);
        m_rtpOverTcp = false;
    }

    public void setupWithTcp(Socket skt) throws IOException {
        reset();
        if (skt == null) {
            throw RTSPSession.IllegalArguementException;
        }
        m_tcpOs = skt.getOutputStream();
        m_rtpOverTcp = true;
    }

    public void reset() {
        m_rtpCount = 0;
        m_lastSeq = 0;
        m_rtpSeq = 0;
    }

    /**
     * @param src 一个Nal unit，不含start code
     *
     * @param length 若大于 {@link #MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP}，
     * 将拆分为多个rtp传输， 即FU-A 分片的单元。 否则采用STAP-A单一时间的组合包，多个Nal单元组成一个RTP包
     *
     * 每次发完清空 m_rtpPayload，数据到达时若 m_rtpPayload.pos > 0 则说明上次之前数据还没发送。
     *
     * StartCode + SPS + StartCode + PPS + StartCode + Nalu1 + StartCode + Nalu2
     * + ... + StartCode + Nalun
     *
     * FU-A: | rtp header(12 bytes) | indicator(1 byte) | FU header(1 byte) |
     * nalu payload |
     *
     * STAP-A: | rtp header(12 bytes) | indicator(1 byte) | next nalu length(2
     * bytes) | next nalu | next nalu length(2 bytes) | next nalu | ...
     *
     * indicator: | F(1) | NRI(2) | Nal Packet Type(5) |
     *
     * if (Nal Packet Type == FU_A)，前三位从 nalu header 前三位获取
     *
     * if (Nal Packet Type == STAP-A)，前三位无所谓（默认0）
     *
     * FU-A header: | S(1) | E(1) | not care(1) | Nal PL Type(5) | 后五位从nalu
     * header 后五位获取
     *
     */
    public void encodeANaluAndSend(byte[] src, int offset, int length) throws IOException {
        if (offset < 0 || length < 0) {
            throw RTSPSession.IllegalArguementException;
        }

        if (length >= MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP) {
            int sizeRemaining = length;
            if (m_rtpPayload.position() > 0) {   // 之前有数据没发送
                m_rtpPayload.flip();
                sendRtpAndClearPayloadBuf();
            }
            // indicator 前三位从nalu header前三位获取
            byte first3bitOfIndicator = (byte) (src[offset] & 0xe0);
            byte indicator = (byte) (first3bitOfIndicator | NaluType.FU_A.value());    // expect = 0x7c
            byte StartFUAHdr = (byte) (src[offset] & 0x1f | StartOrderCodeForFUA);
            byte MidFUAHdr = (byte) (src[offset] & 0x1f | MiddleOrderCodeForFUA);
            byte EndFUAHdr = (byte) (src[offset] & 0x1f | EndOrderCodeForFUA);

            m_rtpPayload.put(indicator);
            m_rtpPayload.put(StartFUAHdr);
            m_rtpPayload.put(src, offset + 1, MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP);
            m_rtpPayload.flip();
            sendRtpAndClearPayloadBuf();
            sizeRemaining -= MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP + 1;
            for (; sizeRemaining > MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP;) {
                m_rtpPayload.put(indicator);
                m_rtpPayload.put(MidFUAHdr);
                m_rtpPayload.put(src, offset + length - sizeRemaining, MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP);
                m_rtpPayload.flip();
                sendRtpAndClearPayloadBuf();
                sizeRemaining -= MAXSIZE_OF_NALUPAYLOAD_PACK_INTO_A_RTP;
            }
            m_rtpPayload.put(indicator);
            m_rtpPayload.put(EndFUAHdr);
            m_rtpPayload.put(src, offset + length - sizeRemaining, sizeRemaining);
            m_rtpPayload.flip();
            sendRtpAndClearPayloadBuf();

        } else {
            if (m_rtpPayload.position() > 0) {   // 之前有数据没发送
                if (m_rtpPayload.remaining() - 2 < length) {    // 当前nalu已装不下，则先发送之前数据，再重新封包
                    m_rtpPayload.flip();
                    sendRtpAndClearPayloadBuf();
                    m_rtpPayload.put(NaluType.STAP_A.value());
                }
            } else {
                m_rtpPayload.put(NaluType.STAP_A.value());
            }
            m_rtpPayload.putShort((short) length);
            m_rtpPayload.put(src, offset, length);

        }

    }

    /**
     * |V(2)|P(1)|X(1)|cc(4)|M(1)|PT(7)|seq(16)|timestamp(32)|SSRC(32)|
     * |<-- header 12 bytes                                        -->| | CSRC (
     * cc * 4 * bytes) ... |(2bytes)|NumOfRemExtHdr(2bytes)| remain extension
     * header ... |<-- ExtHdr, length = (1+NumOfRemExtHdr)*4
     */
    private void genRtpHeader() {
        /**
         * V(rtp version) = 2, P(flag of padding) = 0, X(flag of extension
         * header) = 0, cc(number of CSRC) = 0 M(unknown) = 0, PT(payload type
         * code, 96 for h264) = 96 SSRC 同步源标识符 在同一个RTP会话中不能有两个相同的SSRC值，随机设为9527
         * RTP SSRC是用来标记不同的源的，也就是说，在一个会议中每一个发送者都有一个SSRC CSRC, extension header,
         * padding 都为空
         */
        m_rtpHdr.clear();
        m_rtpHdr.put((byte) (2 << 6));
        m_rtpHdr.put((byte) 96);
        m_rtpSeq++;
        m_rtpHdr.putShort((short) (m_rtpSeq & 0xffff));
        m_rtpHdr.putInt(Calendar.getInstance().get(Calendar.SECOND));
        m_rtpHdr.putInt(9527);
        m_rtpHdr.flip();
    }

    /**
     * send a rtp packet RTP format: | magic number(1 byte: '$') | channel
     * number(1 byte, 0 for RTP, 1 for RTCP) | embedded data length(2 bytes) |
     * data |
     */
    private void sendRtpAndClearPayloadBuf() throws IOException {
        byte NaluTypeCode = (byte) (m_rtpPayload.get(0) & 0x1f);
        if (NaluTypeCode < 32) {
            NaluTypeArray[NaluTypeCode]++;
        } else {
            NaluTypeArray[32]++;
        }
        m_rtpCount++;
        if (m_rtpCount > 200) {
            m_rtpCount = 0;
            System.out.print(".");
            System.out.flush();
        }

        genRtpHeader();
        int seq = m_rtpHdr.getShort(2) & 0xffff;
        if (seq != ++m_lastSeq) {
            System.out.println("[WARN] #" + m_lastSeq + " lost");
            m_lastSeq = seq;
        }

        m_sendBuf.clear();
        if (m_rtpOverTcp) {
            m_sendBuf.put((byte) '$');
            m_sendBuf.put((byte) 0);
            short rtpSize = (short) (m_rtpHdr.remaining() + m_rtpPayload.remaining());
            m_sendBuf.putShort(rtpSize);
//            RTSPPushClient.s_logPw.println("rtp size: " + rtpSize);
        }
        m_sendBuf.put(m_rtpHdr);
        m_sendBuf.put(m_rtpPayload);
        m_sendBuf.flip();

        if (m_rtpOverTcp) {
            m_tcpOs.write(m_sendBuf.array(), 0, m_sendBuf.remaining());
        } else {
            DatagramPacket sendRtpPacket = new DatagramPacket(m_sendBuf.array(), m_sendBuf.remaining(), m_svrAddr, m_StreamServerPort);
            m_udpSkt.send(sendRtpPacket);
        }

        /**
         * 发完后清空 payload ，因为下个nalu数据到达时需要根据 payload 是否有遗留 数据决定是否需要重新封包。
         */
        m_rtpPayload.clear();

    }

    public void close() {
        if (!m_rtpOverTcp) {
            if (m_udpSkt != null && !m_udpSkt.isClosed()) {
                m_udpSkt.close();
            }
        }
    }

    public void PrintTypeCount() {
        System.out.println("Nalu Type Count of RTP:");
        for (int i = 0; i < NaluTypeArray.length; i++) {
            if (NaluTypeArray[i] > 0) {
                System.out.println(i + " " + NaluTypeArray[i]);
            }
        }
        System.out.println("");
    }

}
