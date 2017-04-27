/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package rtsppushclient;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Base64;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author 10256
 */
public class RTSPSession {

    private static final String RTSP_OK = "RTSP/1.0 200 OK";
    private static final String VERSION = " RTSP/1.0";
    private static final String UserAgent = "LeapMotor Push v1.1";

    private static final int STRING_BUFFER_SIZE = 8192;

    public final static RuntimeException IllegalArguementException
            = new RuntimeException("Illegal Arguement");

    private enum EnumRTSPStatus {
        init, options, describe, announce, setup, play, record, pause, teardown
    }

    private String StreamServerIp;
    private int StreamServerRTSPPort;

    private Socket m_socket;
    private InputStream m_is;
    private OutputStream m_os;
    private ByteBuffer m_rwBuf;
    private int m_StreamServerUdpPort;
    private EnumRTSPStatus currRTSPStatus;
    private String m_RTSPUrl;
    private String m_sessionid;
    private int m_seq;
//    private byte[] m_sps;
//    private byte[] m_pps;
    private ByteBuffer m_sps;
    private ByteBuffer m_pps;

    private boolean needTeardown;
    private boolean m_rtpOverTcp;

    public RTSPSession() {
        needTeardown = false;
        m_rwBuf = ByteBuffer.allocate(STRING_BUFFER_SIZE);
        m_socket = null;
        m_is = null;
        m_os = null;
        m_StreamServerUdpPort = -1;
    }

    public Socket getSocket() {
        return m_socket;
    }

    public int getUdpPort() {
        return m_StreamServerUdpPort;
    }

    private void getUrlAndServerInfo(String RTSPUrl) {
        this.m_RTSPUrl = RTSPUrl.trim();
        int ColonIndexAfterIp = -1;
        int SlashIndexAfterPort = -1;
        if (!this.m_RTSPUrl.startsWith("rtsp://")) {
            throw IllegalArguementException;
        }

        if (-1 == (ColonIndexAfterIp = this.m_RTSPUrl.indexOf(':', 7))) {
            throw IllegalArguementException;
        }
        StreamServerIp = this.m_RTSPUrl.substring(7, ColonIndexAfterIp);

        if (-1 == (SlashIndexAfterPort = this.m_RTSPUrl.indexOf('/', ColonIndexAfterIp))) {
            throw IllegalArguementException;
        }

        StreamServerRTSPPort = Integer.parseInt(this.m_RTSPUrl.substring(ColonIndexAfterIp + 1, SlashIndexAfterPort));
    }

    public boolean setupSession(String RTSPUrl, boolean rtpOverTcp) {
        try {
            m_rtpOverTcp = rtpOverTcp;
            currRTSPStatus = EnumRTSPStatus.init;
            m_seq = 1;
            m_sessionid = "";

            getUrlAndServerInfo(RTSPUrl);
            m_socket = new Socket(StreamServerIp, StreamServerRTSPPort);
            m_is = m_socket.getInputStream();
            m_os = m_socket.getOutputStream();

            needTeardown = true;
            if (options() && announce() && setup() && record()) {
                return true;
            }
        } catch (Exception ex) {
            Loger.EXCEPTION(ex);
        }
        return false;
    }

    public void setSpsAndPps(ByteBuffer sps, ByteBuffer pps) {
        if (sps.remaining() == 0 || pps.remaining() == 0) {
            throw new IllegalArgumentException("sps or pps is empty");
        }
        m_sps = ByteBuffer.allocate(sps.remaining());
        m_pps = ByteBuffer.allocate(pps.remaining());
        m_sps.put(sps);
        m_pps.put(pps);
        m_sps.flip();
        m_pps.flip();
    }

    boolean options() throws Exception {
        m_rwBuf.clear();
        StringBuffer buf = new StringBuffer(1024);
        buf.append(EnumRTSPStatus.options.toString().toUpperCase());
        buf.append(" ");
        buf.append(m_RTSPUrl);
        buf.append(" ");
        buf.append(VERSION);
        AppendEOF(buf);
        buf.append("Cseq: ");
        buf.append(m_seq++);
        AppendEOF(buf);
        buf.append("User-Agent: ");
        buf.append(UserAgent);
        AppendEnd(buf, 2);

        return sendAndRead(buf, EnumRTSPStatus.options);
    }

    boolean announce() throws Exception {
        m_rwBuf.clear();
        StringBuffer buf = new StringBuffer(1024);
        buf.append(EnumRTSPStatus.announce.toString().toUpperCase());
        buf.append(" ");
        buf.append(m_RTSPUrl);
        buf.append(" ");
        buf.append(VERSION);
        AppendEOF(buf);
        buf.append("Content-Type: application/sdp");
        AppendEOF(buf);
        buf.append("Cseq: ");
        buf.append(m_seq++);
        AppendEOF(buf);
        buf.append("User-Agent: ");
        buf.append(UserAgent);
        AppendEOF(buf);
        buf.append("Content-Length: ");

        StringBuffer sdp = genSdpInfo();
        buf.append(sdp.length());
        AppendEnd(buf, 2);
        buf.append(sdp);

        return sendAndRead(buf, EnumRTSPStatus.announce);
    }

    boolean setup() throws Exception {
        m_rwBuf.clear();
        StringBuffer buf = new StringBuffer(1024);
        buf.append(EnumRTSPStatus.setup.toString().toUpperCase());
        buf.append(" ");
        buf.append(m_RTSPUrl);
        buf.append("/streamid=0 ");
        buf.append(VERSION);
        AppendEOF(buf);

        buf.append("Transport: RTP/AVP/");
        if (m_rtpOverTcp) {
            buf.append("TCP;unicast;interleaved=0-1;");
        } else {
            /**
             * 由于udp的特点，服务器想发udp给客户端，只能根据客户端连接上来的端口返还回去，发送到客户端指定
             * 的端口是不可达的，因此client_port应该没有意义。但是实测，12854-12855; 31032-31033; 都有问题，
             * 会导致ED解析异常，尝试调试却找不到问题，奇怪的是，以上的测试端口号都是来自ffmpeg测试。 使用ffmpeg
             * rtsp连接也会随机生成一些端口号，我使用ffmpeg生成过的端口号，失败。
             * 202-203可以，但是如果将来某天出问题我也不会太意外。
             */
            buf.append("UDP;unicast;client_port=202-203;");
        }
        buf.append("mode=record");
        AppendEOF(buf);
        buf.append("Cseq: ");
        buf.append(m_seq++);
        AppendEOF(buf);
        buf.append("User-Agent: ");
        buf.append(UserAgent);
        AppendEnd(buf, 2);

        return sendAndRead(buf, EnumRTSPStatus.setup);
    }

    boolean record() throws Exception {
        m_rwBuf.clear();
        StringBuffer buf = new StringBuffer(1024);
        buf.append(EnumRTSPStatus.record.toString().toUpperCase());
        buf.append(" ");
        buf.append(m_RTSPUrl);
        buf.append(" ");
        buf.append(VERSION);
        AppendEOF(buf);
        buf.append("Range: npt=0.000-");
        AppendEOF(buf);
        buf.append("Cseq: ");
        buf.append(m_seq++);
        AppendEOF(buf);
        buf.append("User-Agent: ");
        buf.append(UserAgent);
        AppendEOF(buf);
        buf.append("Session: ");
        buf.append(m_sessionid);
        AppendEnd(buf, 2);

        return sendAndRead(buf, EnumRTSPStatus.record);
    }

    void teardown() throws Exception {
        m_rwBuf.clear();
        StringBuffer buf = new StringBuffer(1024);
        buf.append(EnumRTSPStatus.teardown.toString().toUpperCase());
        buf.append(" ");
        buf.append(m_RTSPUrl);
        buf.append(" ");
        buf.append(VERSION);
        AppendEOF(buf);
        buf.append("Cseq: ");
        buf.append(m_seq++);
        AppendEOF(buf);
        buf.append("User-Agent: ");
        buf.append(UserAgent);
        AppendEOF(buf);
        buf.append("Session: ");
        buf.append(m_sessionid);
        AppendEnd(buf, 2);

        sendAndRead(buf, EnumRTSPStatus.teardown);
    }

    /**
     * 参考自ffmpeg： v=0 o=- 0 0 IN IP4 127.0.0.1 s=No Name c=IN IP4 120.26.86.124
     * t=0 0 a=tool:libavformat 57.40.101 m=video 0 RTP/AVP 96 a=rtpmap:96
     * H264/90000 a=fmtp:96 packetization-mode=1;
     * sprop-parameter-sets=Z2QAH62EAQwgCGEAQwgCGEAQwgCEK1AoAtyA,aO48sA==;
     * profile-level-id=64001F a=control:streamid=0
     */
    private StringBuffer genSdpInfo() {
        StringBuffer sdp = new StringBuffer(1024);
        sdp.append("v=0");
        AppendEOF(sdp);
        sdp.append("o=- 0 0 IN IP4 127.0.0.1");
        AppendEOF(sdp);
        sdp.append("s=No Name");
        AppendEOF(sdp);
        sdp.append("c=IN IP4 ");
        sdp.append(StreamServerIp);
        AppendEOF(sdp);
        sdp.append("t=0 0");
        AppendEOF(sdp);
        sdp.append("a=tool:");
        sdp.append(UserAgent);
        AppendEOF(sdp);
        sdp.append("m=video 0 RTP/AVP 96");
        AppendEOF(sdp);
        sdp.append("a=rtpmap:96 H264/90000");
        AppendEOF(sdp);
        sdp.append("a=fmtp:96 packetization-mode=1; sprop-parameter-sets=");
        sdp.append(Base64.getEncoder().encodeToString(m_sps.array()));
        sdp.append(",");
        sdp.append(Base64.getEncoder().encodeToString(m_pps.array()));
        sdp.append("; profile-level-id=");
        // profile-level-id 为 sps 第2 3 4个字节
        sdp.append(Integer.toHexString(m_sps.get(1) & 0xff));
        sdp.append(Integer.toHexString(m_sps.get(2) & 0xff));
        sdp.append(Integer.toHexString(m_sps.get(3) & 0xff));
        AppendEOF(sdp);
        sdp.append("a=control:streamid=0");
        AppendEOF(sdp);
        return sdp;
    }

    /**
     * @return true if successful, false if response fail or format error
     * @throws IO exception, buffer exception
     */
    private boolean sendAndRead(StringBuffer src, EnumRTSPStatus rtspStatus) throws IOException {
        boolean PrintRTSP = true;
        m_rwBuf.clear();
        m_rwBuf.put(src.toString().getBytes(), 0, src.length());
        m_rwBuf.flip();

        if (PrintRTSP) {
            Loger.INFO("#C->S:");
            Loger.INFO(new String(m_rwBuf.array(), m_rwBuf.position(), m_rwBuf.remaining()).replaceAll("\r\n", "\n") + "\n");
        }
        m_os.write(m_rwBuf.array(), m_rwBuf.position(), m_rwBuf.remaining());
        if (EnumRTSPStatus.teardown == rtspStatus) {
            return true;
        }

        m_rwBuf.clear();
        int n = m_is.read(m_rwBuf.array());

        if (n <= 0) {
            throw new RuntimeException("read fail");
        }
        if (n == m_rwBuf.capacity()) {
            throw new RuntimeException("buffer is not big enough");
        }

        m_rwBuf.limit(n);

        String tmp = new String(m_rwBuf.array(), m_rwBuf.position(), m_rwBuf.remaining());
        if (PrintRTSP) {
            Loger.INFO("#S->C:");
            Loger.INFO(tmp);
            Loger.INFO("");
        }

        if (tmp.contains("Connection: Close")) {
            Loger.INFO("RTSP connection closed");
            needTeardown = false;
            return false;
        }

        if (!tmp.contains(RTSP_OK)) {
            Loger.ERROR("Server return not ok");
            return false;
        }

        if (EnumRTSPStatus.setup == rtspStatus) {
            try {
                m_sessionid = tmp.substring(tmp.indexOf("Session:") + 8, tmp.indexOf("Date:") - 1).trim();
                if (m_sessionid == null && m_sessionid.length() < 0) {
                    Loger.ERROR("RTSPSession: setup format incorrect");
                }
                if (!m_rtpOverTcp) {
                    int indexOfServerPort = tmp.indexOf("server_port=") + 12;
                    String StrUdpPort = tmp.substring(indexOfServerPort, tmp.indexOf("-", indexOfServerPort)).trim();
                    m_StreamServerUdpPort = Integer.parseInt(StrUdpPort);
                }
            } catch (Exception ex) {
                Loger.EXCEPTION(ex);
                return false;
            }
        }
        return true;
    }

    public void close() {
        try {
            if (needTeardown) {
                teardown();
            }
            if (m_os != null) {
                m_os.close();
            }
            if (m_is != null) {
                m_is.close();
            }
            if (m_socket != null) {
                m_socket.close();
            }
        } catch (Exception ex) {
            Loger.EXCEPTION(ex);
        }
    }

    private void AppendEnd(StringBuffer buf, int num) {
        int i = 0;
        for (; i < num; i++) {
            buf.append("\r\n");
        }
    }

    private void AppendEOF(StringBuffer buf) {
        AppendEnd(buf, 1);
    }
}
