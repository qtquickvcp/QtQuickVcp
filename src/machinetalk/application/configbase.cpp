/****************************************************************************
**
** This file was generated by a code generator based on imatix/gsl
** Any changes in this file will be lost.
**
****************************************************************************/
#include "configbase.h"
#include <google/protobuf/text_format.h>
#include "debughelper.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

namespace machinetalk { namespace application {

/** Generic Config Base implementation */
ConfigBase::ConfigBase(QObject *parent)
    : QObject(parent)
    , QQmlParserStatus()
    , m_componentCompleted(false)
    , m_ready(false)
    , m_debugName("Config Base")
    , m_configChannel(nullptr)
    , m_state(Down)
    , m_previousState(Down)
    , m_errorString("")
{
    // initialize config channel
    m_configChannel = new common::RpcClient(this);
    m_configChannel->setDebugName(m_debugName + " - config");
    connect(m_configChannel, &common::RpcClient::socketUriChanged,
            this, &ConfigBase::configUriChanged);
    connect(m_configChannel, &common::RpcClient::stateChanged,
            this, &ConfigBase::configChannelStateChanged);
    connect(m_configChannel, &common::RpcClient::socketMessageReceived,
            this, &ConfigBase::processConfigChannelMessage);

    connect(m_configChannel, &common::RpcClient::heartbeatIntervalChanged,
            this, &ConfigBase::configHeartbeatIntervalChanged);
    // state machine
    connect(this, &ConfigBase::fsmUpEntered,
            this, &ConfigBase::fsmUpEntry);
    connect(this, &ConfigBase::fsmUpExited,
            this, &ConfigBase::fsmUpExit);
    connect(this, &ConfigBase::fsmDownConnect,
            this, &ConfigBase::fsmDownConnectEvent);
    connect(this, &ConfigBase::fsmTryingConfigUp,
            this, &ConfigBase::fsmTryingConfigUpEvent);
    connect(this, &ConfigBase::fsmTryingDisconnect,
            this, &ConfigBase::fsmTryingDisconnectEvent);
    connect(this, &ConfigBase::fsmListingApplicationRetrieved,
            this, &ConfigBase::fsmListingApplicationRetrievedEvent);
    connect(this, &ConfigBase::fsmListingConfigTrying,
            this, &ConfigBase::fsmListingConfigTryingEvent);
    connect(this, &ConfigBase::fsmListingDisconnect,
            this, &ConfigBase::fsmListingDisconnectEvent);
    connect(this, &ConfigBase::fsmUpConfigTrying,
            this, &ConfigBase::fsmUpConfigTryingEvent);
    connect(this, &ConfigBase::fsmUpLoadApplication,
            this, &ConfigBase::fsmUpLoadApplicationEvent);
    connect(this, &ConfigBase::fsmUpDisconnect,
            this, &ConfigBase::fsmUpDisconnectEvent);
    connect(this, &ConfigBase::fsmLoadingApplicationLoaded,
            this, &ConfigBase::fsmLoadingApplicationLoadedEvent);
    connect(this, &ConfigBase::fsmLoadingConfigTrying,
            this, &ConfigBase::fsmLoadingConfigTryingEvent);
    connect(this, &ConfigBase::fsmLoadingDisconnect,
            this, &ConfigBase::fsmLoadingDisconnectEvent);
}

ConfigBase::~ConfigBase()
{
}

void ConfigBase::startConfigChannel()
{
    m_configChannel->setReady(true);
}

void ConfigBase::stopConfigChannel()
{
    m_configChannel->setReady(false);
}

/** Processes all message received on config */
void ConfigBase::processConfigChannelMessage(const Container &rx)
{

    // react to describe application message
    if (rx.type() == MT_DESCRIBE_APPLICATION)
    {

        if (m_state == Listing)
        {
            emit fsmListingApplicationRetrieved(QPrivateSignal());
        }
        describeApplicationReceived(rx);
    }

    // react to application detail message
    if (rx.type() == MT_APPLICATION_DETAIL)
    {

        if (m_state == Loading)
        {
            emit fsmLoadingApplicationLoaded(QPrivateSignal());
        }
        applicationDetailReceived(rx);
    }

    // react to error message
    if (rx.type() == MT_ERROR)
    {

        // update error string with note
        m_errorString = "";
        for (int i = 0; i < rx.note_size(); ++i)
        {
            m_errorString.append(QString::fromStdString(rx.note(i)) + "\n");
        }
        emit errorStringChanged(m_errorString);
    }

    emit configMessageReceived(rx);
}

void ConfigBase::sendConfigMessage(ContainerType type, Container &tx)
{
    m_configChannel->sendSocketMessage(type, tx);
    if (type == MT_RETRIEVE_APPLICATION)
    {

        if (m_state == Up)
        {
            emit fsmUpLoadApplication(QPrivateSignal());
        }
    }
}

void ConfigBase::sendListApplications()
{
    Container &tx = m_configTx;
    sendConfigMessage(MT_LIST_APPLICATIONS, tx);
}

void ConfigBase::sendRetrieveApplication(Container &tx)
{
    sendConfigMessage(MT_RETRIEVE_APPLICATION, tx);
}

void ConfigBase::fsmDown()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State DOWN");
#endif
    m_state = Down;
    emit stateChanged(m_state);
}

void ConfigBase::fsmDownConnectEvent()
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
        startConfigChannel();
     }
}

void ConfigBase::fsmTrying()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State TRYING");
#endif
    m_state = Trying;
    emit stateChanged(m_state);
}

void ConfigBase::fsmTryingConfigUpEvent()
{
    if (m_state == Trying)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONFIG UP");
#endif
        // handle state change
        emit fsmTryingExited(QPrivateSignal());
        fsmListing();
        emit fsmListingEntered(QPrivateSignal());
        // execute actions
        sendListApplications();
     }
}

void ConfigBase::fsmTryingDisconnectEvent()
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
        stopConfigChannel();
     }
}

void ConfigBase::fsmListing()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State LISTING");
#endif
    m_state = Listing;
    emit stateChanged(m_state);
}

void ConfigBase::fsmListingApplicationRetrievedEvent()
{
    if (m_state == Listing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event APPLICATION RETRIEVED");
#endif
        // handle state change
        emit fsmListingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
     }
}

void ConfigBase::fsmListingConfigTryingEvent()
{
    if (m_state == Listing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONFIG TRYING");
#endif
        // handle state change
        emit fsmListingExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void ConfigBase::fsmListingDisconnectEvent()
{
    if (m_state == Listing)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmListingExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopConfigChannel();
     }
}

void ConfigBase::fsmUp()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State UP");
#endif
    m_state = Up;
    emit stateChanged(m_state);
}
void ConfigBase::fsmUpEntry()
{
    syncConfig();
}
void ConfigBase::fsmUpExit()
{
    unsyncConfig();
}

void ConfigBase::fsmUpConfigTryingEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONFIG TRYING");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void ConfigBase::fsmUpLoadApplicationEvent()
{
    if (m_state == Up)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event LOAD APPLICATION");
#endif
        // handle state change
        emit fsmUpExited(QPrivateSignal());
        fsmLoading();
        emit fsmLoadingEntered(QPrivateSignal());
        // execute actions
     }
}

void ConfigBase::fsmUpDisconnectEvent()
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
        stopConfigChannel();
     }
}

void ConfigBase::fsmLoading()
{
#ifdef QT_DEBUG
    DEBUG_TAG(1, m_debugName, "State LOADING");
#endif
    m_state = Loading;
    emit stateChanged(m_state);
}

void ConfigBase::fsmLoadingApplicationLoadedEvent()
{
    if (m_state == Loading)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event APPLICATION LOADED");
#endif
        // handle state change
        emit fsmLoadingExited(QPrivateSignal());
        fsmUp();
        emit fsmUpEntered(QPrivateSignal());
        // execute actions
     }
}

void ConfigBase::fsmLoadingConfigTryingEvent()
{
    if (m_state == Loading)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event CONFIG TRYING");
#endif
        // handle state change
        emit fsmLoadingExited(QPrivateSignal());
        fsmTrying();
        emit fsmTryingEntered(QPrivateSignal());
        // execute actions
     }
}

void ConfigBase::fsmLoadingDisconnectEvent()
{
    if (m_state == Loading)
    {
#ifdef QT_DEBUG
        DEBUG_TAG(1, m_debugName, "Event DISCONNECT");
#endif
        // handle state change
        emit fsmLoadingExited(QPrivateSignal());
        fsmDown();
        emit fsmDownEntered(QPrivateSignal());
        // execute actions
        stopConfigChannel();
     }
}

void ConfigBase::configChannelStateChanged(common::RpcClient::State state)
{

    if (state == common::RpcClient::Trying)
    {
        if (m_state == Listing)
        {
            emit fsmListingConfigTrying(QPrivateSignal());
        }
        if (m_state == Up)
        {
            emit fsmUpConfigTrying(QPrivateSignal());
        }
        if (m_state == Loading)
        {
            emit fsmLoadingConfigTrying(QPrivateSignal());
        }
    }

    if (state == common::RpcClient::Up)
    {
        if (m_state == Trying)
        {
            emit fsmTryingConfigUp(QPrivateSignal());
        }
    }
}

/** start trigger function */
void ConfigBase::start()
{
    if (m_state == Down) {
        emit fsmDownConnect(QPrivateSignal());
    }
}

/** stop trigger function */
void ConfigBase::stop()
{
    if (m_state == Trying) {
        emit fsmTryingDisconnect(QPrivateSignal());
    }
    if (m_state == Listing) {
        emit fsmListingDisconnect(QPrivateSignal());
    }
    if (m_state == Up) {
        emit fsmUpDisconnect(QPrivateSignal());
    }
    if (m_state == Loading) {
        emit fsmLoadingDisconnect(QPrivateSignal());
    }
}

} } // namespace machinetalk::application
