/****************************************************************************
**
** This file was generated by a code generator based on imatix/gsl
** Any changes in this file will be lost.
**
****************************************************************************/
#include "launchersubscribe.h"
#include <google/protobuf/text_format.h>
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace machinetalk {
namespace application {

/** Generic Launcher Subscribe implementation */
LauncherSubscribe::LauncherSubscribe(QObject *parent) :
    QObject(parent),
    m_ready(false),
    m_debugName("Launcher Subscribe"),
    m_socketUri(""),
    m_context(nullptr),
    m_socket(nullptr),
    m_state(Down),
    m_previousState(Down),
    m_errorString("")
    ,m_heartbeatTimer(new QTimer(this)),
    m_heartbeatInterval(2500),
    m_heartbeatLiveness(0),
    m_heartbeatResetLiveness(2)
{

    m_heartbeatTimer->setSingleShot(true);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &LauncherSubscribe::heartbeatTimerTick);
    // state machine
    connect(this, &LauncherSubscribe::fsmDownConnect,
            this, &LauncherSubscribe::fsmDownConnectEvent);
    connect(this, &LauncherSubscribe::fsmTryingConnected,
            this, &LauncherSubscribe::fsmTryingConnectedEvent);
    connect(this, &LauncherSubscribe::fsmTryingDisconnect,
            this, &LauncherSubscribe::fsmTryingDisconnectEvent);
    connect(this, &LauncherSubscribe::fsmUpTimeout,
            this, &LauncherSubscribe::fsmUpTimeoutEvent);
    connect(this, &LauncherSubscribe::fsmUpTick,
            this, &LauncherSubscribe::fsmUpTickEvent);
    connect(this, &LauncherSubscribe::fsmUpMessageReceived,
            this, &LauncherSubscribe::fsmUpMessageReceivedEvent);
    connect(this, &LauncherSubscribe::fsmUpDisconnect,
            this, &LauncherSubscribe::fsmUpDisconnectEvent);

    m_context = new PollingZMQContext(this, 1);
    connect(m_context, &PollingZMQContext::pollError,
            this, &LauncherSubscribe::socketError);
    m_context->start();
}

LauncherSubscribe::~LauncherSubscribe()
{
    stopSocket();

    if (m_context != nullptr)
    {
        m_context->stop();
        m_context->deleteLater();
        m_context = nullptr;
    }
}

/** Add a topic that should be subscribed **/
void LauncherSubscribe::addSocketTopic(const QString &name)
{
    m_socketTopics.insert(name);
}

/** Removes a topic from the list of topics that should be subscribed **/
void LauncherSubscribe::removeSocketTopic(const QString &name)
{
    m_socketTopics.remove(name);
}

/** Clears the the topics that should be subscribed **/
void LauncherSubscribe::clearSocketTopics()
{
    m_socketTopics.clear();
}

/** Connects the 0MQ sockets */
bool LauncherSubscribe::startSocket()
{
    m_socket = m_context->createSocket(ZMQSocket::TYP_SUB, this);
    m_socket->setLinger(0);

    try {
        m_socket->connectTo(m_socketUri);
    }
    catch (const zmq::error_t &e) {
        QString errorString;
        errorString = QString("Error %1: ").arg(e.num()) + QString(e.what());
        //updateState(SocketError, errorString); TODO
        return false;
    }

    connect(m_socket, &ZMQSocket::messageReceived,
            this, &LauncherSubscribe::processSocketMessage);


    foreach(QString topic, m_socketTopics)
    {
        m_socket->subscribeTo(topic.toLocal8Bit());
    }

#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "sockets connected" << m_socketUri);
#endif

    return true;
}

/** Disconnects the 0MQ sockets */
void LauncherSubscribe::stopSocket()
{
    if (m_socket != nullptr)
    {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void LauncherSubscribe::resetHeartbeatLiveness()
{
    m_heartbeatLiveness = m_heartbeatResetLiveness;
}

void LauncherSubscribe::resetHeartbeatTimer()
{
    if (m_heartbeatTimer->isActive())
    {
        m_heartbeatTimer->stop();
    }

    if (m_heartbeatInterval > 0)
    {
        m_heartbeatTimer->setInterval(m_heartbeatInterval);
        m_heartbeatTimer->start();
    }
}

void LauncherSubscribe::startHeartbeatTimer()
{
    resetHeartbeatTimer();
}

void LauncherSubscribe::stopHeartbeatTimer()
{
    m_heartbeatTimer->stop();
}

void LauncherSubscribe::heartbeatTimerTick()
{
    m_heartbeatLiveness -= 1;
    if (m_heartbeatLiveness == 0)
    {
         if (m_state == Up)
         {
             emit fsmUpTimeout(QPrivateSignal());
         }
         return;
    }
    if (m_state == Up)
    {
        emit fsmUpTick(QPrivateSignal());
    }
}

/** Processes all message received on socket */
void LauncherSubscribe::processSocketMessage(const QList<QByteArray> &messageList)
{
    Container &rx = m_socketRx;
    QByteArray topic;

    if (messageList.length() < 2)  // in case we received insufficient data
    {
        return;
    }

    // we only handle the first two messges
    topic = messageList.at(0);
    rx.ParseFromArray(messageList.at(1).data(), messageList.at(1).size());

#ifdef QT_DEBUG
    std::string s;
    gpb::TextFormat::PrintToString(rx, &s);
    DEBUG_TAG(3, m_debugName, "server message" << QString::fromStdString(s));
#endif

    // react to any incoming message

    if (m_state == Up)
    {
        emit fsmUpMessageReceived(QPrivateSignal());
    }

    // react to ping message
    if (rx.type() == MT_PING)
    {
        return; // ping is uninteresting
    }

    // react to launcher full update message
    if (rx.type() == MT_LAUNCHER_FULL_UPDATE)
    {
        if (rx.has_pparams())
        {
            ProtocolParameters pparams = rx.pparams();
            m_heartbeatInterval = pparams.keepalive_timer();
        }

        if (m_state == Trying)
        {
            emit fsmTryingConnected(QPrivateSignal());
        }
    }

    emit socketMessageReceived(topic, rx);
}

void LauncherSubscribe::socketError(int errorNum, const QString &errorMsg)
{
    QString errorString;
    errorString = QString("Error %1: ").arg(errorNum) + errorMsg;
}

void LauncherSubscribe::fsmDown()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
    m_state = Down;
    emit stateChanged(m_state);
}

void LauncherSubscribe::fsmDownConnectEvent()
{
    if (m_state == Down)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONNECT");
#endif
        // handle state change
        emit fsmDownExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
        startSocket();
     }
}

void LauncherSubscribe::fsmTrying()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State TRYING");
#endif
    m_state = Trying;
    emit stateChanged(m_state);
}

void LauncherSubscribe::fsmTryingConnectedEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONNECTED");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
        resetHeartbeatLiveness();
        startHeartbeatTimer();
     }
}

void LauncherSubscribe::fsmTryingDisconnectEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopHeartbeatTimer();
        stopSocket();
     }
}

void LauncherSubscribe::fsmUp()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State UP");
#endif
    m_state = Up;
    emit stateChanged(m_state);
}

void LauncherSubscribe::fsmUpTimeoutEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event TIMEOUT");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
        stopHeartbeatTimer();
        stopSocket();
        startSocket();
     }
}

void LauncherSubscribe::fsmUpTickEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event TICK");
#endif
        // execute actions
        resetHeartbeatTimer();
     }
}

void LauncherSubscribe::fsmUpMessageReceivedEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event MESSAGE RECEIVED");
#endif
        // execute actions
        resetHeartbeatLiveness();
        resetHeartbeatTimer();
     }
}

void LauncherSubscribe::fsmUpDisconnectEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopHeartbeatTimer();
        stopSocket();
     }
}

/** start trigger function */
void LauncherSubscribe::start()
{
    if (m_state == Down) {
        emit fsmDownConnect(QPrivateSignal());
    }
}

/** stop trigger function */
void LauncherSubscribe::stop()
{
    if (m_state == Trying) {
        emit fsmTryingDisconnect(QPrivateSignal());
    }
    if (m_state == Up) {
        emit fsmUpDisconnect(QPrivateSignal());
    }
}
} // namespace application
} // namespace machinetalk
