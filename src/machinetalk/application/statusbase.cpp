/****************************************************************************
**
** This file was generated by a code generator based on imatix/gsl
** Any changes in this file will be lost.
**
****************************************************************************/
#include "statusbase.h"
#include <google/protobuf/text_format.h>
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace machinetalk { namespace application {

/** Generic Status Base implementation */
StatusBase::StatusBase(QObject *parent)
    : QObject(parent)
    , QQmlParserStatus()
    , m_componentCompleted(false)
    , m_ready(false)
    , m_debugName("Status Base")
    , m_statusChannel(nullptr)
    , m_state(Down)
    , m_previousState(Down)
    , m_errorString("")
{
    // initialize status channel
    m_statusChannel = new application::StatusSubscribe(this);
    m_statusChannel->setDebugName(m_debugName + " - status");
    connect(m_statusChannel, &application::StatusSubscribe::socketUriChanged,
            this, &StatusBase::statusUriChanged);
    connect(m_statusChannel, &application::StatusSubscribe::stateChanged,
            this, &StatusBase::statusChannelStateChanged);
    connect(m_statusChannel, &application::StatusSubscribe::socketMessageReceived,
            this, &StatusBase::processStatusChannelMessage);

    connect(m_statusChannel, &application::StatusSubscribe::heartbeatIntervalChanged,
            this, &StatusBase::statusHeartbeatIntervalChanged);
    // state machine
    connect(this, &StatusBase::fsmUpEntered,
            this, &StatusBase::fsmUpEntry);
    connect(this, &StatusBase::fsmUpExited,
            this, &StatusBase::fsmUpExit);
    connect(this, &StatusBase::fsmDownConnect,
            this, &StatusBase::fsmDownConnectEvent);
    connect(this, &StatusBase::fsmTryingStatusUp,
            this, &StatusBase::fsmTryingStatusUpEvent);
    connect(this, &StatusBase::fsmTryingDisconnect,
            this, &StatusBase::fsmTryingDisconnectEvent);
    connect(this, &StatusBase::fsmSyncingChannelsSynced,
            this, &StatusBase::fsmSyncingChannelsSyncedEvent);
    connect(this, &StatusBase::fsmSyncingStatusTrying,
            this, &StatusBase::fsmSyncingStatusTryingEvent);
    connect(this, &StatusBase::fsmSyncingDisconnect,
            this, &StatusBase::fsmSyncingDisconnectEvent);
    connect(this, &StatusBase::fsmUpStatusTrying,
            this, &StatusBase::fsmUpStatusTryingEvent);
    connect(this, &StatusBase::fsmUpDisconnect,
            this, &StatusBase::fsmUpDisconnectEvent);
}

StatusBase::~StatusBase()
{
}

/** Add a topic that should be subscribed **/
void StatusBase::addStatusTopic(const QString &name)
{
    m_statusChannel->addSocketTopic(name);
}

/** Removes a topic from the list of topics that should be subscribed **/
void StatusBase::removeStatusTopic(const QString &name)
{
    m_statusChannel->removeSocketTopic(name);
}

/** Clears the the topics that should be subscribed **/
void StatusBase::clearStatusTopics()
{
    m_statusChannel->clearSocketTopics();
}

void StatusBase::startStatusChannel()
{
    m_statusChannel->setReady(true);
}

void StatusBase::stopStatusChannel()
{
    m_statusChannel->setReady(false);
}

/** Processes all message received on status */
void StatusBase::processStatusChannelMessage(const QByteArray &topic, const Container &rx)
{

    // react to emcstat full update message
    if (rx.type() == MT_EMCSTAT_FULL_UPDATE)
    {
        emcstatFullUpdateReceived(topic, rx);
    }

    // react to emcstat incremental update message
    if (rx.type() == MT_EMCSTAT_INCREMENTAL_UPDATE)
    {
        emcstatIncrementalUpdateReceived(topic, rx);
    }

    emit statusMessageReceived(topic, rx);
}

void StatusBase::fsmDown()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
    m_state = Down;
    emit stateChanged(m_state);
}

void StatusBase::fsmDownConnectEvent()
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
        updateTopics();
        startStatusChannel();
     }
}

void StatusBase::fsmTrying()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State TRYING");
#endif
    m_state = Trying;
    emit stateChanged(m_state);
}

void StatusBase::fsmTryingStatusUpEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event STATUS UP");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmSyncing();
        emit fsmSyncingEntered(QPrivateSignal());
        // execute actions
     }
}

void StatusBase::fsmTryingDisconnectEvent()
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
        stopStatusChannel();
     }
}

void StatusBase::fsmSyncing()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State SYNCING");
#endif
    m_state = Syncing;
    emit stateChanged(m_state);
}

void StatusBase::fsmSyncingChannelsSyncedEvent()
{
    if (m_state == Syncing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CHANNELS SYNCED");
#endif
        // handle state change
        emit fsmSyncingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
     }
}

void StatusBase::fsmSyncingStatusTryingEvent()
{
    if (m_state == Syncing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event STATUS TRYING");
#endif
        // handle state change
        emit fsmSyncingExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void StatusBase::fsmSyncingDisconnectEvent()
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
        stopStatusChannel();
     }
}

void StatusBase::fsmUp()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State UP");
#endif
    m_state = Up;
    emit stateChanged(m_state);
}
void StatusBase::fsmUpEntry()
{
    syncStatus();
}
void StatusBase::fsmUpExit()
{
    unsyncStatus();
}

void StatusBase::fsmUpStatusTryingEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event STATUS TRYING");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void StatusBase::fsmUpDisconnectEvent()
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
        stopStatusChannel();
     }
}

void StatusBase::statusChannelStateChanged(application::StatusSubscribe::State state)
{

    if (state == application::StatusSubscribe::Trying)
    {
        if (m_state == Up)
        {
            emit fsmUpStatusTrying(QPrivateSignal());
        }
    }

    if (state == application::StatusSubscribe::Trying)
    {
        if (m_state == Syncing)
        {
            emit fsmSyncingStatusTrying(QPrivateSignal());
        }
    }

    if (state == application::StatusSubscribe::Up)
    {
        if (m_state == Trying)
        {
            emit fsmTryingStatusUp(QPrivateSignal());
        }
    }
}

/** start trigger function */
void StatusBase::start()
{
    if (m_state == Down) {
        emit fsmDownConnect(QPrivateSignal());
    }
}

/** stop trigger function */
void StatusBase::stop()
{
    if (m_state == Trying) {
        emit fsmTryingDisconnect(QPrivateSignal());
    }
    if (m_state == Up) {
        emit fsmUpDisconnect(QPrivateSignal());
    }
}

/** channels synced trigger function */
void StatusBase::channelsSynced()
{
    if (m_state == Syncing) {
        emit fsmSyncingChannelsSynced(QPrivateSignal());
    }
}

} } // namespace machinetalk::application
