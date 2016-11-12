/****************************************************************************
**
** This code was generated by a code generator based on imatix/gsl
** Any changes in this code will be lost.
**
****************************************************************************/
#include "remotecomponentbase.h"
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace halremote {

/** Generic Remote Component Base implementation */
RemoteComponentBase::RemoteComponentBase(QObject *parent) :
    QObject(parent),
    QQmlParserStatus(),
    m_componentCompleted(false),
    m_ready(false),
    m_debugName("Remote Component Base"),
    m_halrcmdChannel(nullptr),
    m_halrcompChannel(nullptr),
    m_state(Down),
    m_previousState(Down),
    m_fsm(nullptr),
    m_errorString("")
{
    // initialize halrcmd channel
    m_halrcmdChannel = new machinetalk::RpcClient(this);
    m_halrcmdChannel->setDebugName(m_debugName + " - halrcmd");
    connect(m_halrcmdChannel, &machinetalk::RpcClient::socketUriChanged,
            this, &RemoteComponentBase::halrcmdUriChanged);
    connect(m_halrcmdChannel, &machinetalk::RpcClient::stateChanged,
            this, &RemoteComponentBase::halrcmdChannelStateChanged);
    connect(m_halrcmdChannel, &machinetalk::RpcClient::socketMessageReceived,
            this, &RemoteComponentBase::processHalrcmdChannelMessage);
    // initialize halrcomp channel
    m_halrcompChannel = new halremote::HalrcompSubscribe(this);
    m_halrcompChannel->setDebugName(m_debugName + " - halrcomp");
    connect(m_halrcompChannel, &halremote::HalrcompSubscribe::socketUriChanged,
            this, &RemoteComponentBase::halrcompUriChanged);
    connect(m_halrcompChannel, &halremote::HalrcompSubscribe::stateChanged,
            this, &RemoteComponentBase::halrcompChannelStateChanged);
    connect(m_halrcompChannel, &halremote::HalrcompSubscribe::socketMessageReceived,
            this, &RemoteComponentBase::processHalrcompChannelMessage);

    connect(m_halrcmdChannel, &machinetalk::RpcClient::heartbeatIntervalChanged,
            this, &RemoteComponentBase::halrcmdHeartbeatIntervalChanged);

    connect(m_halrcompChannel, &halremote::HalrcompSubscribe::heartbeatIntervalChanged,
            this, &RemoteComponentBase::halrcompHeartbeatIntervalChanged);

    m_fsm = new QStateMachine(this);
    QState *downState = new QState(m_fsm);
    connect(downState, &QState::entered, this, &RemoteComponentBase::fsmDownEntered, Qt::QueuedConnection);
    connect(downState, &QState::entered, this, &RemoteComponentBase::setDisconnected, Qt::QueuedConnection);
    connect(downState, &QState::exited, this, &RemoteComponentBase::setConnecting, Qt::QueuedConnection);
    QState *tryingState = new QState(m_fsm);
    connect(tryingState, &QState::entered, this, &RemoteComponentBase::fsmTryingEntered, Qt::QueuedConnection);
    QState *bindState = new QState(m_fsm);
    connect(bindState, &QState::entered, this, &RemoteComponentBase::fsmBindEntered, Qt::QueuedConnection);
    QState *bindingState = new QState(m_fsm);
    connect(bindingState, &QState::entered, this, &RemoteComponentBase::fsmBindingEntered, Qt::QueuedConnection);
    QState *syncingState = new QState(m_fsm);
    connect(syncingState, &QState::entered, this, &RemoteComponentBase::fsmSyncingEntered, Qt::QueuedConnection);
    QState *syncedState = new QState(m_fsm);
    connect(syncedState, &QState::entered, this, &RemoteComponentBase::fsmSyncedEntered, Qt::QueuedConnection);
    connect(syncedState, &QState::entered, this, &RemoteComponentBase::setConnected, Qt::QueuedConnection);
    QState *errorState = new QState(m_fsm);
    connect(errorState, &QState::entered, this, &RemoteComponentBase::fsmErrorEntered, Qt::QueuedConnection);
    connect(errorState, &QState::entered, this, &RemoteComponentBase::setError, Qt::QueuedConnection);
    m_fsm->setInitialState(downState);
    m_fsm->start();

    connect(this, &RemoteComponentBase::fsmDownConnect,
            this, &RemoteComponentBase::fsmDownConnectQueued, Qt::QueuedConnection);
    downState->addTransition(this, &RemoteComponentBase::fsmDownConnectQueued, tryingState);
    connect(this, &RemoteComponentBase::fsmTryingHalrcmdUp,
            this, &RemoteComponentBase::fsmTryingHalrcmdUpQueued, Qt::QueuedConnection);
    tryingState->addTransition(this, &RemoteComponentBase::fsmTryingHalrcmdUpQueued, bindState);
    connect(this, &RemoteComponentBase::fsmTryingDisconnect,
            this, &RemoteComponentBase::fsmTryingDisconnectQueued, Qt::QueuedConnection);
    tryingState->addTransition(this, &RemoteComponentBase::fsmTryingDisconnectQueued, downState);
    connect(this, &RemoteComponentBase::fsmBindHalrcompBindMsgSent,
            this, &RemoteComponentBase::fsmBindHalrcompBindMsgSentQueued, Qt::QueuedConnection);
    bindState->addTransition(this, &RemoteComponentBase::fsmBindHalrcompBindMsgSentQueued, bindingState);
    connect(this, &RemoteComponentBase::fsmBindNoBind,
            this, &RemoteComponentBase::fsmBindNoBindQueued, Qt::QueuedConnection);
    bindState->addTransition(this, &RemoteComponentBase::fsmBindNoBindQueued, syncingState);
    connect(this, &RemoteComponentBase::fsmBindingBindConfirmed,
            this, &RemoteComponentBase::fsmBindingBindConfirmedQueued, Qt::QueuedConnection);
    bindingState->addTransition(this, &RemoteComponentBase::fsmBindingBindConfirmedQueued, syncingState);
    connect(this, &RemoteComponentBase::fsmBindingBindRejected,
            this, &RemoteComponentBase::fsmBindingBindRejectedQueued, Qt::QueuedConnection);
    bindingState->addTransition(this, &RemoteComponentBase::fsmBindingBindRejectedQueued, errorState);
    connect(this, &RemoteComponentBase::fsmBindingHalrcmdTrying,
            this, &RemoteComponentBase::fsmBindingHalrcmdTryingQueued, Qt::QueuedConnection);
    bindingState->addTransition(this, &RemoteComponentBase::fsmBindingHalrcmdTryingQueued, tryingState);
    connect(this, &RemoteComponentBase::fsmBindingDisconnect,
            this, &RemoteComponentBase::fsmBindingDisconnectQueued, Qt::QueuedConnection);
    bindingState->addTransition(this, &RemoteComponentBase::fsmBindingDisconnectQueued, downState);
    connect(this, &RemoteComponentBase::fsmSyncingHalrcmdTrying,
            this, &RemoteComponentBase::fsmSyncingHalrcmdTryingQueued, Qt::QueuedConnection);
    syncingState->addTransition(this, &RemoteComponentBase::fsmSyncingHalrcmdTryingQueued, tryingState);
    connect(this, &RemoteComponentBase::fsmSyncingHalrcompUp,
            this, &RemoteComponentBase::fsmSyncingHalrcompUpQueued, Qt::QueuedConnection);
    syncingState->addTransition(this, &RemoteComponentBase::fsmSyncingHalrcompUpQueued, syncedState);
    connect(this, &RemoteComponentBase::fsmSyncingSyncFailed,
            this, &RemoteComponentBase::fsmSyncingSyncFailedQueued, Qt::QueuedConnection);
    syncingState->addTransition(this, &RemoteComponentBase::fsmSyncingSyncFailedQueued, errorState);
    connect(this, &RemoteComponentBase::fsmSyncingDisconnect,
            this, &RemoteComponentBase::fsmSyncingDisconnectQueued, Qt::QueuedConnection);
    syncingState->addTransition(this, &RemoteComponentBase::fsmSyncingDisconnectQueued, downState);
    connect(this, &RemoteComponentBase::fsmSyncedHalrcompTrying,
            this, &RemoteComponentBase::fsmSyncedHalrcompTryingQueued, Qt::QueuedConnection);
    syncedState->addTransition(this, &RemoteComponentBase::fsmSyncedHalrcompTryingQueued, syncingState);
    connect(this, &RemoteComponentBase::fsmSyncedHalrcmdTrying,
            this, &RemoteComponentBase::fsmSyncedHalrcmdTryingQueued, Qt::QueuedConnection);
    syncedState->addTransition(this, &RemoteComponentBase::fsmSyncedHalrcmdTryingQueued, tryingState);
    connect(this, &RemoteComponentBase::fsmSyncedSetRejected,
            this, &RemoteComponentBase::fsmSyncedSetRejectedQueued, Qt::QueuedConnection);
    syncedState->addTransition(this, &RemoteComponentBase::fsmSyncedSetRejectedQueued, errorState);
    connect(this, &RemoteComponentBase::fsmSyncedHalrcompSetMsgSent,
            this, &RemoteComponentBase::fsmSyncedHalrcompSetMsgSentQueued, Qt::QueuedConnection);
    syncedState->addTransition(this, &RemoteComponentBase::fsmSyncedHalrcompSetMsgSentQueued, syncedState);
    connect(this, &RemoteComponentBase::fsmSyncedDisconnect,
            this, &RemoteComponentBase::fsmSyncedDisconnectQueued, Qt::QueuedConnection);
    syncedState->addTransition(this, &RemoteComponentBase::fsmSyncedDisconnectQueued, downState);
    connect(this, &RemoteComponentBase::fsmErrorDisconnect,
            this, &RemoteComponentBase::fsmErrorDisconnectQueued, Qt::QueuedConnection);
    errorState->addTransition(this, &RemoteComponentBase::fsmErrorDisconnectQueued, downState);

    connect(this, &RemoteComponentBase::fsmDownConnect,
            this, &RemoteComponentBase::fsmDownConnectEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmTryingHalrcmdUp,
            this, &RemoteComponentBase::fsmTryingHalrcmdUpEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmTryingDisconnect,
            this, &RemoteComponentBase::fsmTryingDisconnectEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmBindHalrcompBindMsgSent,
            this, &RemoteComponentBase::fsmBindHalrcompBindMsgSentEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmBindNoBind,
            this, &RemoteComponentBase::fsmBindNoBindEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmBindingBindConfirmed,
            this, &RemoteComponentBase::fsmBindingBindConfirmedEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmBindingBindRejected,
            this, &RemoteComponentBase::fsmBindingBindRejectedEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmBindingHalrcmdTrying,
            this, &RemoteComponentBase::fsmBindingHalrcmdTryingEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmBindingDisconnect,
            this, &RemoteComponentBase::fsmBindingDisconnectEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncingHalrcmdTrying,
            this, &RemoteComponentBase::fsmSyncingHalrcmdTryingEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncingHalrcompUp,
            this, &RemoteComponentBase::fsmSyncingHalrcompUpEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncingSyncFailed,
            this, &RemoteComponentBase::fsmSyncingSyncFailedEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncingDisconnect,
            this, &RemoteComponentBase::fsmSyncingDisconnectEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncedHalrcompTrying,
            this, &RemoteComponentBase::fsmSyncedHalrcompTryingEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncedHalrcmdTrying,
            this, &RemoteComponentBase::fsmSyncedHalrcmdTryingEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncedSetRejected,
            this, &RemoteComponentBase::fsmSyncedSetRejectedEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncedHalrcompSetMsgSent,
            this, &RemoteComponentBase::fsmSyncedHalrcompSetMsgSentEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmSyncedDisconnect,
            this, &RemoteComponentBase::fsmSyncedDisconnectEvent, Qt::QueuedConnection);
    connect(this, &RemoteComponentBase::fsmErrorDisconnect,
            this, &RemoteComponentBase::fsmErrorDisconnectEvent, Qt::QueuedConnection);
}

RemoteComponentBase::~RemoteComponentBase()
{
}

/** Add a topic that should be subscribed **/
void RemoteComponentBase::addHalrcompTopic(const QString &name)
{
    m_halrcompChannel->addSocketTopic(name);
}

/** Removes a topic from the list of topics that should be subscribed **/
void RemoteComponentBase::removeHalrcompTopic(const QString &name)
{
    m_halrcompChannel->removeSocketTopic(name);
}

/** Clears the the topics that should be subscribed **/
void RemoteComponentBase::clearHalrcompTopics()
{
    m_halrcompChannel->clearSocketTopics();
}

void RemoteComponentBase::startHalrcmdChannel()
{
    m_halrcmdChannel->setReady(true);
}

void RemoteComponentBase::stopHalrcmdChannel()
{
    m_halrcmdChannel->setReady(false);
}

void RemoteComponentBase::startHalrcompChannel()
{
    m_halrcompChannel->setReady(true);
}

void RemoteComponentBase::stopHalrcompChannel()
{
    m_halrcompChannel->setReady(false);
}

/** Processes all message received on halrcmd */
void RemoteComponentBase::processHalrcmdChannelMessage(const pb::Container &rx)
{

    // react to halrcomp bind confirm message
    if (rx.type() == pb::MT_HALRCOMP_BIND_CONFIRM)
    {

        if (m_state == Binding)
        {
            emit fsmBindingBindConfirmed();
        }
    }

    // react to halrcomp bind reject message
    if (rx.type() == pb::MT_HALRCOMP_BIND_REJECT)
    {

        // update error string with note
        m_errorString = "";
        for (int i = 0; i < rx.note_size(); ++i)
        {
            m_errorString.append(QString::fromStdString(rx.note(i)) + "\n");
        }
        emit errorStringChanged(m_errorString);

        if (m_state == Binding)
        {
            emit fsmBindingBindRejected();
        }
    }

    // react to halrcomp set reject message
    if (rx.type() == pb::MT_HALRCOMP_SET_REJECT)
    {

        // update error string with note
        m_errorString = "";
        for (int i = 0; i < rx.note_size(); ++i)
        {
            m_errorString.append(QString::fromStdString(rx.note(i)) + "\n");
        }
        emit errorStringChanged(m_errorString);

        if (m_state == Synced)
        {
            emit fsmSyncedSetRejected();
        }
    }

    emit halrcmdMessageReceived(rx);
}

/** Processes all message received on halrcomp */
void RemoteComponentBase::processHalrcompChannelMessage(const QByteArray &topic, const pb::Container &rx)
{

    // react to halrcomp full update message
    if (rx.type() == pb::MT_HALRCOMP_FULL_UPDATE)
    {
        halrcompFullUpdateReceived(topic, rx);
    }

    // react to halrcomp incremental update message
    if (rx.type() == pb::MT_HALRCOMP_INCREMENTAL_UPDATE)
    {
        halrcompIncrementalUpdateReceived(topic, rx);
    }

    // react to halrcomp error message
    if (rx.type() == pb::MT_HALRCOMP_ERROR)
    {

        // update error string with note
        m_errorString = "";
        for (int i = 0; i < rx.note_size(); ++i)
        {
            m_errorString.append(QString::fromStdString(rx.note(i)) + "\n");
        }
        emit errorStringChanged(m_errorString);

        if (m_state == Syncing)
        {
            emit fsmSyncingSyncFailed();
        }
        halrcompErrorReceived(topic, rx);
    }

    emit halrcompMessageReceived(topic, rx);
}

void RemoteComponentBase::sendHalrcmdMessage(pb::ContainerType type, pb::Container &tx)
{
    m_halrcmdChannel->sendSocketMessage(type, tx);
    if (type == pb::MT_HALRCOMP_BIND)
    {

        if (m_state == Bind)
        {
            emit fsmBindHalrcompBindMsgSent();
        }
    }
    if (type == pb::MT_HALRCOMP_SET)
    {

        if (m_state == Synced)
        {
            emit fsmSyncedHalrcompSetMsgSent();
        }
    }
}

void RemoteComponentBase::sendHalrcompBind(pb::Container &tx)
{
    sendHalrcmdMessage(pb::MT_HALRCOMP_BIND, tx);
}

void RemoteComponentBase::sendHalrcompSet(pb::Container &tx)
{
    sendHalrcmdMessage(pb::MT_HALRCOMP_SET, tx);
}

void RemoteComponentBase::fsmDownEntered()
{
    if (m_previousState != Down)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
        m_previousState = Down;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmDownConnectEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event CONNECT");
#endif

    m_state = Trying;
    addPins();
    startHalrcmdChannel();
}

void RemoteComponentBase::fsmTryingEntered()
{
    if (m_previousState != Trying)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State TRYING");
#endif
        m_previousState = Trying;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmTryingHalrcmdUpEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCMD UP");
#endif

    m_state = Bind;
    bindComponent();
}

void RemoteComponentBase::fsmTryingDisconnectEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif

    m_state = Down;
    stopHalrcmdChannel();
    stopHalrcompChannel();
    removePins();
}

void RemoteComponentBase::fsmBindEntered()
{
    if (m_previousState != Bind)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State BIND");
#endif
        m_previousState = Bind;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmBindHalrcompBindMsgSentEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCOMP BIND MSG SENT");
#endif

    m_state = Binding;
}

void RemoteComponentBase::fsmBindNoBindEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event NO BIND");
#endif

    m_state = Syncing;
    startHalrcompChannel();
}

void RemoteComponentBase::fsmBindingEntered()
{
    if (m_previousState != Binding)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State BINDING");
#endif
        m_previousState = Binding;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmBindingBindConfirmedEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event BIND CONFIRMED");
#endif

    m_state = Syncing;
    startHalrcompChannel();
}

void RemoteComponentBase::fsmBindingBindRejectedEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event BIND REJECTED");
#endif

    m_state = Error;
    stopHalrcmdChannel();
}

void RemoteComponentBase::fsmBindingHalrcmdTryingEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCMD TRYING");
#endif

    m_state = Trying;
}

void RemoteComponentBase::fsmBindingDisconnectEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif

    m_state = Down;
    stopHalrcmdChannel();
    stopHalrcompChannel();
    removePins();
}

void RemoteComponentBase::fsmSyncingEntered()
{
    if (m_previousState != Syncing)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State SYNCING");
#endif
        m_previousState = Syncing;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmSyncingHalrcmdTryingEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCMD TRYING");
#endif

    m_state = Trying;
    stopHalrcompChannel();
}

void RemoteComponentBase::fsmSyncingHalrcompUpEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCOMP UP");
#endif

    m_state = Synced;
}

void RemoteComponentBase::fsmSyncingSyncFailedEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event SYNC FAILED");
#endif

    m_state = Error;
    stopHalrcompChannel();
    stopHalrcmdChannel();
}

void RemoteComponentBase::fsmSyncingDisconnectEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif

    m_state = Down;
    stopHalrcmdChannel();
    stopHalrcompChannel();
    removePins();
}

void RemoteComponentBase::fsmSyncedEntered()
{
    if (m_previousState != Synced)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State SYNCED");
#endif
        m_previousState = Synced;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmSyncedHalrcompTryingEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCOMP TRYING");
#endif

    m_state = Syncing;
    unsyncPins();
    setTimeout();
}

void RemoteComponentBase::fsmSyncedHalrcmdTryingEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCMD TRYING");
#endif

    m_state = Trying;
    stopHalrcompChannel();
    unsyncPins();
    setTimeout();
}

void RemoteComponentBase::fsmSyncedSetRejectedEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event SET REJECTED");
#endif

    m_state = Error;
    stopHalrcompChannel();
    stopHalrcmdChannel();
}

void RemoteComponentBase::fsmSyncedHalrcompSetMsgSentEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event HALRCOMP SET MSG SENT");
#endif

    m_state = Synced;
}

void RemoteComponentBase::fsmSyncedDisconnectEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif

    m_state = Down;
    stopHalrcmdChannel();
    stopHalrcompChannel();
    removePins();
}

void RemoteComponentBase::fsmErrorEntered()
{
    if (m_previousState != Error)
    {
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State ERROR");
#endif
        m_previousState = Error;
        emit stateChanged(m_state);
    }
}

void RemoteComponentBase::fsmErrorDisconnectEvent()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif

    m_state = Down;
    stopHalrcmdChannel();
    stopHalrcompChannel();
    removePins();
}

void RemoteComponentBase::halrcmdChannelStateChanged(machinetalk::RpcClient::State state)
{

    if (state == machinetalk::RpcClient::Trying)
    {
        if (m_state == Syncing)
        {
            emit fsmSyncingHalrcmdTrying();
        }
        if (m_state == Synced)
        {
            emit fsmSyncedHalrcmdTrying();
        }
        if (m_state == Binding)
        {
            emit fsmBindingHalrcmdTrying();
        }
    }

    if (state == machinetalk::RpcClient::Up)
    {
        if (m_state == Trying)
        {
            emit fsmTryingHalrcmdUp();
        }
    }
}

void RemoteComponentBase::halrcompChannelStateChanged(halremote::HalrcompSubscribe::State state)
{

    if (state == halremote::HalrcompSubscribe::Trying)
    {
        if (m_state == Synced)
        {
            emit fsmSyncedHalrcompTrying();
        }
    }

    if (state == halremote::HalrcompSubscribe::Up)
    {
        if (m_state == Syncing)
        {
            emit fsmSyncingHalrcompUp();
        }
    }
}

/** no bind trigger */
void RemoteComponentBase::noBind()
{
    if (m_state == Bind) {
        emit fsmBindNoBind();
    }
}

/** start trigger */
void RemoteComponentBase::start()
{
    if (m_state == Down) {
        emit fsmDownConnect();
    }
}

/** stop trigger */
void RemoteComponentBase::stop()
{
    if (m_state == Trying) {
        emit fsmTryingDisconnect();
    }
    if (m_state == Binding) {
        emit fsmBindingDisconnect();
    }
    if (m_state == Syncing) {
        emit fsmSyncingDisconnect();
    }
    if (m_state == Synced) {
        emit fsmSyncedDisconnect();
    }
    if (m_state == Error) {
        emit fsmErrorDisconnect();
    }
}
}; // namespace halremote
