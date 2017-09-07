/****************************************************************************
**
** This file was generated by a code generator based on imatix/gsl
** Any changes in this file will be lost.
**
****************************************************************************/
#include "paramserver.h"
#include <google/protobuf/text_format.h>
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace machinetalk { namespace param {

/** Generic Param Server implementation */
ParamServer::ParamServer(QObject *parent)
    : QObject(parent)
    , QQmlParserStatus()
    , m_componentCompleted(false)
    , m_ready(false)
    , m_debugName("Param Server")
    , m_paramcmdChannel(nullptr)
    , m_paramChannel(nullptr)
    , m_state(State::Down)
    , m_previousState(State::Down)
    , m_errorString("")
{
    // initialize paramcmd channel
    m_paramcmdChannel = new common::RpcService(this);
    m_paramcmdChannel->setDebugName(m_debugName + " - paramcmd");
    connect(m_paramcmdChannel, &common::RpcService::socketUriChanged,
            this, &ParamServer::paramcmdUriChanged);
    connect(m_paramcmdChannel, &common::RpcService::socketMessageReceived,
            this, &ParamServer::processParamcmdChannelMessage);
    // initialize param channel
    m_paramChannel = new common::Publish(this);
    m_paramChannel->setDebugName(m_debugName + " - param");
    connect(m_paramChannel, &common::Publish::socketUriChanged,
            this, &ParamServer::paramUriChanged);

    connect(m_paramChannel, &common::Publish::heartbeatIntervalChanged,
            this, &ParamServer::paramHeartbeatIntervalChanged);
    // state machine
    connect(this, &ParamServer::fsmDownConnect,
            this, &ParamServer::fsmDownConnectEvent);
    connect(this, &ParamServer::fsmUpDisconnect,
            this, &ParamServer::fsmUpDisconnectEvent);
}

ParamServer::~ParamServer()
{
}

void ParamServer::startParamcmdChannel()
{
    m_paramcmdChannel->setReady(true);
}

void ParamServer::stopParamcmdChannel()
{
    m_paramcmdChannel->setReady(false);
}

void ParamServer::startParamChannel()
{
    m_paramChannel->setReady(true);
}

void ParamServer::stopParamChannel()
{
    m_paramChannel->setReady(false);
}

/** Processes all message received on paramcmd */
void ParamServer::processParamcmdChannelMessage(const QByteArray &topic, const Container &rx)
{

    // react to incremental update message
    if (rx.type() == MT_INCREMENTAL_UPDATE)
    {
        incrementalUpdateReceived(topic, rx);
    }

    emit paramcmdMessageReceived(topic, rx);
}

void ParamServer::sendParamMessage(const QByteArray &topic, ContainerType type, Container &tx)
{
    m_paramChannel->sendSocketMessage(topic, type, tx);
}

void ParamServer::sendFullUpdate(const QByteArray &topic, Container &tx)
{
    sendParamMessage(topic, MT_FULL_UPDATE, tx);
}

void ParamServer::sendIncrementalUpdate(const QByteArray &topic, Container &tx)
{
    sendParamMessage(topic, MT_INCREMENTAL_UPDATE, tx);
}

void ParamServer::fsmDown()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
    m_state = State::Down;
    emit stateChanged(m_state);
}

void ParamServer::fsmDownConnectEvent()
{
    if (m_state == State::Down)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONNECT");
#endif
        // handle state change
        emit fsmDownExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
        startParamcmdChannel();
        startParamChannel();
     }
}

void ParamServer::fsmUp()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State UP");
#endif
    m_state = State::Up;
    emit stateChanged(m_state);
}

void ParamServer::fsmUpDisconnectEvent()
{
    if (m_state == State::Up)
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
     }
}

} } // namespace machinetalk::param
