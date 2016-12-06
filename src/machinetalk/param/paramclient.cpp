/****************************************************************************
**
** This file was generated by a code generator based on imatix/gsl
** Any changes in this file will be lost.
**
****************************************************************************/
#include "paramclient.h"
#include <google/protobuf/text_format.h>
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace machinetalk {
namespace param {

/** Generic Param Client implementation */
ParamClient::ParamClient(QObject *parent) :
    QObject(parent),
    QQmlParserStatus(),
    m_componentCompleted(false),
    m_ready(false),
    m_debugName("Param Client"),
    m_paramcmdChannel(nullptr),
    m_paramChannel(nullptr),
    m_state(Down),
    m_previousState(Down),
    m_errorString("")
{
    // initialize paramcmd channel
    m_paramcmdChannel = new common::RpcClient(this);
    m_paramcmdChannel->setDebugName(m_debugName + " - paramcmd");
    connect(m_paramcmdChannel, &common::RpcClient::socketUriChanged,
            this, &ParamClient::paramcmdUriChanged);
    connect(m_paramcmdChannel, &common::RpcClient::stateChanged,
            this, &ParamClient::paramcmdChannelStateChanged);
    // initialize param channel
    m_paramChannel = new common::Subscribe(this);
    m_paramChannel->setDebugName(m_debugName + " - param");
    connect(m_paramChannel, &common::Subscribe::socketUriChanged,
            this, &ParamClient::paramUriChanged);
    connect(m_paramChannel, &common::Subscribe::stateChanged,
            this, &ParamClient::paramChannelStateChanged);
    connect(m_paramChannel, &common::Subscribe::socketMessageReceived,
            this, &ParamClient::processParamChannelMessage);

    connect(m_paramcmdChannel, &common::RpcClient::heartbeatIntervalChanged,
            this, &ParamClient::paramcmdHeartbeatIntervalChanged);

    connect(m_paramChannel, &common::Subscribe::heartbeatIntervalChanged,
            this, &ParamClient::paramHeartbeatIntervalChanged);
    // state machine
    connect(this, &ParamClient::fsmUpEntered,
            this, &ParamClient::fsmUpEntry);
    connect(this, &ParamClient::fsmUpExited,
            this, &ParamClient::fsmUpExit);
    connect(this, &ParamClient::fsmDownConnect,
            this, &ParamClient::fsmDownConnectEvent);
    connect(this, &ParamClient::fsmConnectingParamcmdUp,
            this, &ParamClient::fsmConnectingParamcmdUpEvent);
    connect(this, &ParamClient::fsmConnectingParamUp,
            this, &ParamClient::fsmConnectingParamUpEvent);
    connect(this, &ParamClient::fsmConnectingDisconnect,
            this, &ParamClient::fsmConnectingDisconnectEvent);
    connect(this, &ParamClient::fsmSyncingParamUp,
            this, &ParamClient::fsmSyncingParamUpEvent);
    connect(this, &ParamClient::fsmSyncingParamcmdTrying,
            this, &ParamClient::fsmSyncingParamcmdTryingEvent);
    connect(this, &ParamClient::fsmSyncingDisconnect,
            this, &ParamClient::fsmSyncingDisconnectEvent);
    connect(this, &ParamClient::fsmTryingParamcmdUp,
            this, &ParamClient::fsmTryingParamcmdUpEvent);
    connect(this, &ParamClient::fsmTryingParamTrying,
            this, &ParamClient::fsmTryingParamTryingEvent);
    connect(this, &ParamClient::fsmTryingDisconnect,
            this, &ParamClient::fsmTryingDisconnectEvent);
    connect(this, &ParamClient::fsmUpParamcmdTrying,
            this, &ParamClient::fsmUpParamcmdTryingEvent);
    connect(this, &ParamClient::fsmUpParamTrying,
            this, &ParamClient::fsmUpParamTryingEvent);
    connect(this, &ParamClient::fsmUpDisconnect,
            this, &ParamClient::fsmUpDisconnectEvent);
}

ParamClient::~ParamClient()
{
}

/** Add a topic that should be subscribed **/
void ParamClient::addParamTopic(const QString &name)
{
    m_paramChannel->addSocketTopic(name);
}

/** Removes a topic from the list of topics that should be subscribed **/
void ParamClient::removeParamTopic(const QString &name)
{
    m_paramChannel->removeSocketTopic(name);
}

/** Clears the the topics that should be subscribed **/
void ParamClient::clearParamTopics()
{
    m_paramChannel->clearSocketTopics();
}

void ParamClient::startParamcmdChannel()
{
    m_paramcmdChannel->setReady(true);
}

void ParamClient::stopParamcmdChannel()
{
    m_paramcmdChannel->setReady(false);
}

void ParamClient::startParamChannel()
{
    m_paramChannel->setReady(true);
}

void ParamClient::stopParamChannel()
{
    m_paramChannel->setReady(false);
}

/** Processes all message received on param */
void ParamClient::processParamChannelMessage(const QByteArray &topic, const Container &rx)
{

    // react to full update message
    if (rx.type() == MT_FULL_UPDATE)
    {
        fullUpdateReceived(topic, rx);
    }

    // react to incremental update message
    if (rx.type() == MT_INCREMENTAL_UPDATE)
    {
        incrementalUpdateReceived(topic, rx);
    }

    emit paramMessageReceived(topic, rx);
}

void ParamClient::sendParamcmdMessage(ContainerType type, Container &tx)
{
    m_paramcmdChannel->sendSocketMessage(type, tx);
}

void ParamClient::sendIncrementalUpdate(Container &tx)
{
    sendParamcmdMessage(MT_INCREMENTAL_UPDATE, tx);
}

void ParamClient::fsmDown()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
    m_state = Down;
    emit stateChanged(m_state);
}

void ParamClient::fsmDownConnectEvent()
{
    if (m_state == Down)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONNECT");
#endif
        // handle state change
        emit fsmDownExited(QPrivateSignal());
        fsmConnecting();
        emit fsmConnectingEntered(QPrivateSignal());
        // execute actions
        startParamcmdChannel();
        startParamChannel();
     }
}

void ParamClient::fsmConnecting()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State CONNECTING");
#endif
    m_state = Connecting;
    emit stateChanged(m_state);
}

void ParamClient::fsmConnectingParamcmdUpEvent()
{
    if (m_state == Connecting)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAMCMD UP");
#endif
        // handle state change
        emit fsmConnectingExited(QPrivateSignal());
        fsmSyncing();
        emit fsmSyncingEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmConnectingParamUpEvent()
{
    if (m_state == Connecting)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAM UP");
#endif
        // handle state change
        emit fsmConnectingExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmConnectingDisconnectEvent()
{
    if (m_state == Connecting)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmConnectingExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopParamcmdChannel();
        stopParamChannel();
        removeKeys();
     }
}

void ParamClient::fsmSyncing()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State SYNCING");
#endif
    m_state = Syncing;
    emit stateChanged(m_state);
}

void ParamClient::fsmSyncingParamUpEvent()
{
    if (m_state == Syncing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAM UP");
#endif
        // handle state change
        emit fsmSyncingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmSyncingParamcmdTryingEvent()
{
    if (m_state == Syncing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAMCMD TRYING");
#endif
        // handle state change
        emit fsmSyncingExited(QPrivateSignal());
        fsmConnecting();
        emit fsmConnectingEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmSyncingDisconnectEvent()
{
    if (m_state == Syncing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmSyncingExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopParamcmdChannel();
        stopParamChannel();
        removeKeys();
     }
}

void ParamClient::fsmTrying()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State TRYING");
#endif
    m_state = Trying;
    emit stateChanged(m_state);
}

void ParamClient::fsmTryingParamcmdUpEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAMCMD UP");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmTryingParamTryingEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAM TRYING");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmConnecting();
        emit fsmConnectingEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmTryingDisconnectEvent()
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
        stopParamcmdChannel();
        stopParamChannel();
        removeKeys();
     }
}

void ParamClient::fsmUp()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State UP");
#endif
    m_state = Up;
    emit stateChanged(m_state);
}
void ParamClient::fsmUpEntry()
{
    setSynced();
}
void ParamClient::fsmUpExit()
{
    clearSynced();
    unsyncKeys();
}

void ParamClient::fsmUpParamcmdTryingEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAMCMD TRYING");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmUpParamTryingEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event PARAM TRYING");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmSyncing();
        emit fsmSyncingEntered(QPrivateSignal());
        // execute actions
     }
}

void ParamClient::fsmUpDisconnectEvent()
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
        stopParamcmdChannel();
        stopParamChannel();
        removeKeys();
     }
}

void ParamClient::paramcmdChannelStateChanged(common::RpcClient::State state)
{

    if (state == common::RpcClient::Trying)
    {
        if (m_state == Syncing)
        {
            emit fsmSyncingParamcmdTrying(QPrivateSignal());
        }
        if (m_state == Up)
        {
            emit fsmUpParamcmdTrying(QPrivateSignal());
        }
    }

    if (state == common::RpcClient::Up)
    {
        if (m_state == Trying)
        {
            emit fsmTryingParamcmdUp(QPrivateSignal());
        }
        if (m_state == Connecting)
        {
            emit fsmConnectingParamcmdUp(QPrivateSignal());
        }
    }
}

void ParamClient::paramChannelStateChanged(common::Subscribe::State state)
{

    if (state == common::Subscribe::Trying)
    {
        if (m_state == Trying)
        {
            emit fsmTryingParamTrying(QPrivateSignal());
        }
        if (m_state == Up)
        {
            emit fsmUpParamTrying(QPrivateSignal());
        }
    }

    if (state == common::Subscribe::Up)
    {
        if (m_state == Syncing)
        {
            emit fsmSyncingParamUp(QPrivateSignal());
        }
        if (m_state == Connecting)
        {
            emit fsmConnectingParamUp(QPrivateSignal());
        }
    }
}

/** start trigger function */
void ParamClient::start()
{
    if (m_state == Down) {
        emit fsmDownConnect(QPrivateSignal());
    }
}

/** stop trigger function */
void ParamClient::stop()
{
    if (m_state == Connecting) {
        emit fsmConnectingDisconnect(QPrivateSignal());
    }
    if (m_state == Syncing) {
        emit fsmSyncingDisconnect(QPrivateSignal());
    }
    if (m_state == Trying) {
        emit fsmTryingDisconnect(QPrivateSignal());
    }
    if (m_state == Up) {
        emit fsmUpDisconnect(QPrivateSignal());
    }
}
} // namespace param
} // namespace machinetalk
