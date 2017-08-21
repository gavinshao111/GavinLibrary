/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package rtsppushclient;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 *
 * @author 10256
 */
public class RTSPPushClient {

    public static final byte[] NaluStartCode = new byte[]{0, 0, 0, 1};
    public static final int MaxNaluSize = 1024 * 1024;
    public static byte SPSTYPE = 7;
    public static byte PPSTYPE = 8;

    private final RTSPSession m_rtspSession;
    private final RTPStream m_rtpStream;
    private String m_rtspUrl;
    private ByteBuffer m_sps;
    private ByteBuffer m_pps;
    private boolean m_rtpOverTcp;

    public RTSPPushClient() {
        m_rtspUrl = null;
        m_rtspSession = new RTSPSession();
        m_rtpStream = new RTPStream();
        m_sps = null;
        m_pps = null;
        m_rtpOverTcp = true;    // use tcp default
    }

    /**
     * @param RTSPUrl Expected to be like
     * rtsp://120.26.86.124:8888/realtime/1234/1/realtime.sdp
     */
    public void setUrl(String RTSPUrl) {
        m_rtspUrl = RTSPUrl;
    }

    /**
     * @param rtpOverTcp Use tcp or udp to transport rtp packet.
     */
    public void setTransport(boolean rtpOverTcp) {
        m_rtpOverTcp = rtpOverTcp;
    }

    /**
     * If sps and pps are all set, setup rtsp connection and pack source data
     * into rtp packet and send to stream server, else source data will be
     * ignored.
     *
     * @param src The array from which bytes are to be read as a frame which
     * exclude start code(0 0 0 1)
     *
     * @param offset The offset within the array of the first byte to be read;
     * must be non-negative and no larger than <tt>array.length</tt>
     *
     * @param length The number of bytes to be read from the given array; must
     * be non-negative and no larger than
     * <tt>array.length - offset</tt>
     *
     * @return True if send successful, false if fail. If return false, you
     * should stop to call this function and call close() finally.
     */
    public boolean pushStream(byte src[], int offset, int length) {
        if (offset < 0 || length + offset > src.length || length > MaxNaluSize) {
            return false;
        }

        if (m_sps == null || m_pps == null) {
            if ((src[offset] & 0x1f) == SPSTYPE) {
                m_sps = ByteBuffer.allocate(length);
                m_sps.put(src, offset, length);
                m_sps.flip();
            }

            if ((src[offset] & 0x1f) == PPSTYPE) {
                m_pps = ByteBuffer.allocate(length);
                m_pps.put(src, offset, length);
                m_pps.flip();
            }

            if (m_sps == null || m_pps == null) {
                return true;
            }

            if (!setupRTSPSession()) {
                return false;
            }
            try {
                m_rtpStream.setupWithTcp(m_rtspSession.getSocket());
            } catch (IOException e) {
                Loger.EXCEPTION(e);
                return false;
            }
            return sendStream(m_sps) && sendStream(m_pps);

        } else {
            return sendStream(src, offset, length);
        }
    }

    public void close() {
        m_rtpStream.close();
        m_rtspSession.close();
        m_sps = null;
        m_pps = null;
    }

    private boolean setupRTSPSession() {
        m_rtspSession.setSpsAndPps(m_sps, m_pps);
        m_sps.rewind();
        m_pps.rewind();

        if (!m_rtspSession.setupSession(m_rtspUrl, m_rtpOverTcp)) {
            return false;
        }

        if (!m_rtpOverTcp) {
            if (m_rtspSession.getUdpPort() <= 0) {
                Loger.DEBUG("Illegal udp port");
                return false;
            }
        }
        return true;
    }

    private boolean sendStream(byte src[], int offset, int length) {
        try {
            m_rtpStream.encodeANaluAndSend(src, offset, length);
            return true;
        } catch (IOException ex) {
            Loger.EXCEPTION(ex);
            return false;
        }

    }

    private boolean sendStream(ByteBuffer src) {
        Loger.DEBUG("sendStream ByteBuffer position:" + src.position() + " remaining:" + src.remaining() + " len:" + src.array().length);
        return sendStream(src.array(), src.position(), src.remaining());
    }

    /**
     * find next start code(0001), and return the offset after that.
     *
     * @return -1 if doesn't contain any start coode
     */
    public static int getOffsetAfterNextStartCode(byte src[], int offset, int length) {
        int i = offset;
        for (; i < length; i++) {
            if (src[i] == 0
                    && src[i + 1] == 0
                    && src[i + 2] == 0
                    && src[i + 3] == 1) {
                return i + 4;
            }
        }
        return -1;
    }

    /**
     * find next start code(0001) start from current position + offset in next
     * length of bytes, and return the offset after next start code and
     * src.position move to the position after next start code
     */
    public static int getOffsetAfterNextStartCode(ByteBuffer src, int offset, int length) {
        if (length + offset > src.remaining()) {
            throw new RuntimeException("Illegal Arguement");
        }

        int rc = getOffsetAfterNextStartCode(src.array(), offset + src.position(), length);
        if (rc == -1) {
            return -1;
        }
        src.position(rc);
        return rc;
    }

    /**
     * find next start code(0001) start from current position in remaining
     * bytes, and return the offset after next start code and src.position move
     * to the position after next start code
     */
    public static int getOffsetAfterNextStartCode(ByteBuffer src) {
        return getOffsetAfterNextStartCode(src, 0, src.remaining());
    }

}
