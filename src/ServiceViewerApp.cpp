//--------------------------------------------------------------------------------------
//
//  File:       ServiceViewerApp.cpp
//
//  Project:    M+M
//
//  Contains:   The class definition for the service viewer application class.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by HPlus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms,
//              with or without modification, are permitted provided that the following
//              conditions are met:
//                * Redistributions of source code must retain the above copyright
//                  notice, this list of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright
//                  notice, this list of conditions and the following disclaimer in the
//                  documentation and/or other materials provided with the
//                  distribution.
//                * Neither the name of the copyright holders nor the names of its
//                  contributors may be used to endorse or promote products derived
//                  from this software without specific prior written permission.
//
//              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//              "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//              LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//              PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//              OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//              SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//              LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//              DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//              THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//              (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//              OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  Created:    2014-05-08
//
//--------------------------------------------------------------------------------------

#include "ServiceViewerApp.h"
#include "BackgroundScanner.h"
#include "ServiceEntity.h"

//#include "ODEnableLogging.h"
#include "ODLogging.h"

#include "ofAppRunner.h"
#include "ofBitmapFont.h"
#include "ofGraphics.h"
#include "ofMesh.h"

#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/energybased/FMMMLayout.h>

// Note that openFrameworks defines a macro called 'check' :( which messes up other header files.
#undef check
#include "M+MAdapterChannel.h"
#include "M+MUtilities.h"

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wc++11-extensions"
# pragma clang diagnostic ignored "-Wdocumentation"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# pragma clang diagnostic ignored "-Wpadded"
# pragma clang diagnostic ignored "-Wshadow"
# pragma clang diagnostic ignored "-Wunused-parameter"
# pragma clang diagnostic ignored "-Wweak-vtables"
#endif // defined(__APPLE__)
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 
 @brief The class definition for the service viewer application class. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The minimum time between background scans. */
static const float kMinScanInterval = 5;

/*! @brief The line width for a normal connection. */
static const float kNormalConnectionWidth = 2;

/*! @brief The line width for a normal connection. */
static const float kServiceConnectionWidth = (2 * kNormalConnectionWidth);

/*! @brief @c true if the port direction resources are available. */
static bool                             lPortsValid = false;
/*! @brief The port used to determine if a port being checked can be used as an output. */
static MplusM::Common::AdapterChannel * lInputOnlyPort = NULL;
/*! @brief The port used to determine if a port being checked can be used as an input. */
static MplusM::Common::AdapterChannel * lOutputOnlyPort = NULL;

/*! @brief The name of the port used to determine if a port being checked can be used as an output. */
static yarp::os::ConstString lInputOnlyPortName;
/*! @brief The name of the port used to determine if a port being checked can be used as an input. */
static yarp::os::ConstString lOutputOnlyPortName;

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

/*! @brief Create the resources needed to determine port directions. */
static void createDirectionTestPorts(void)
{
    OD_LOG_ENTER();//####
    lInputOnlyPortName = MplusM::Common::GetRandomChannelName("/checkdirection/channel_");
    lOutputOnlyPortName = MplusM::Common::GetRandomChannelName("/checkdirection/channel_");
    lInputOnlyPort = new MplusM::Common::AdapterChannel(false);
    if (lInputOnlyPort)
    {
        lInputOnlyPort->setInputMode(true);
        lInputOnlyPort->setOutputMode(false);
        lOutputOnlyPort = new MplusM::Common::AdapterChannel(true);
        if (lOutputOnlyPort)
        {
            lOutputOnlyPort->setInputMode(false);
            lOutputOnlyPort->setOutputMode(true);
            if (lInputOnlyPort->openWithRetries(lInputOnlyPortName, STANDARD_WAIT_TIME) &&
                lOutputOnlyPort->openWithRetries(lOutputOnlyPortName, STANDARD_WAIT_TIME))
            {
                lPortsValid = true;
            }
        }
    }
    OD_LOG_EXIT();//####
} // createDirectionTestPorts

/*! @brief Release the resources used to determine port directions. */
static void destroyDirectionTestPorts(void)
{
    OD_LOG_ENTER();//####
    if (lInputOnlyPort)
    {
#if defined(MpM_DoExplicitClose)
        lInputOnlyPort->close();
#endif // defined(MpM_DoExplicitClose)
        MplusM::Common::AdapterChannel::RelinquishChannel(lInputOnlyPort);
    }
    if (lOutputOnlyPort)
    {
#if defined(MpM_DoExplicitClose)
        lOutputOnlyPort->close();
#endif // defined(MpM_DoExplicitClose)
        MplusM::Common::AdapterChannel::RelinquishChannel(lOutputOnlyPort);
    }
    lPortsValid = false;
    OD_LOG_EXIT();//####
} // destroyDirectionTestPorts

/*! @brief Determine whether a port can be used for input and/or output.
 @param portName The name of the port to check.
 @returns The allowed directions for the port. */
static PortEntry::PortDirection determinePortDirection(const yarp::os::ConstString & portName)
{
    OD_LOG_ENTER();//####
    OD_LOG_S1("portName = ", portName.c_str());//####
    PortEntry::PortDirection result = PortEntry::kPortDirectionUnknown;
    
    if (lPortsValid)
    {
        bool canDoInput = false;
        bool canDoOutput = false;

        // First, check if we are looking at a client port - because of how they are constructed, attempting to connect
        // to them will result in a hang, so we just treat them as I/O.
        switch (MplusM::Utilities::GetPortKind(portName))
        {
            case MplusM::Utilities::kPortKindClient:
                canDoInput = canDoOutput = true;
                break;
                
            case MplusM::Utilities::kPortKindService:
            case MplusM::Utilities::kPortKindServiceRegistry:
                canDoInput = true;
                break;
                
            default:
                // Determine by doing a test connection.
                if (MplusM::Common::NetworkConnectWithRetries(lOutputOnlyPortName, portName, STANDARD_WAIT_TIME, false))
                {
                    canDoInput = true;
                    if (! MplusM::Common::NetworkDisconnectWithRetries(lOutputOnlyPortName, portName,
                                                                       STANDARD_WAIT_TIME))
                    {
                        OD_LOG("(! MplusM::Common::NetworkDisconnectWithRetries(lOutputOnlyPortName, portName, "//####
                               "STANDARD_WAIT_TIME))");//####
                    }
                }
                if (MplusM::Common::NetworkConnectWithRetries(portName, lInputOnlyPortName, STANDARD_WAIT_TIME, false))
                {
                    canDoOutput = true;
                    if (! MplusM::Common::NetworkDisconnectWithRetries(portName, lInputOnlyPortName,
                                                                       STANDARD_WAIT_TIME))
                    {
                        OD_LOG("(! MplusM::Common::NetworkDisconnectWithRetries(portName, lInputOnlyPortName, "//####
                               "STANDARD_WAIT_TIME))");//####
                    }
                }
                break;
                
        }
        if (canDoInput)
        {
            result = (canDoOutput ? PortEntry::kPortDirectionInputOutput : PortEntry::kPortDirectionInput);
        }
        else if (canDoOutput)
        {
            result = PortEntry::kPortDirectionOutput;
        }
    }
    OD_LOG_EXIT_L(static_cast<long>(result));//####
    return result;
} // determinePortDirection

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and destructors
#endif // defined(__APPLE__)

ServiceViewerApp::ServiceViewerApp(void) :
            inherited(), _entities1(), _entities2(), _ports1(), _ports2(), _backgroundEntities(&_entities1),
            _foregroundEntities(&_entities2), _backgroundPorts(&_ports1), _foregroundPorts(&_ports2),
            _firstAddPort(NULL), _firstRemovePort(NULL), _scanner(new BackgroundScanner(*this, kMinScanInterval)),
            _addingUDPConnection(false), _addIsActive(false), _altActive(false), _commandActive(false),
            _controlActive(false), _dragActive(false), _movementActive(false), _networkAvailable(false),
            _registryAvailable(false), _removeIsActive(false), _shiftActive(false)
{
    OD_LOG_ENTER();//####
    OD_LOG_EXIT_P(this);//####
} // ServiceViewerApp::ServiceViewerApp

#if defined(__APPLE__)
# pragma mark Actions
#endif // defined(__APPLE__)

void ServiceViewerApp::addEntityToBackground(ServiceEntity * anEntity)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("anEntity = ", anEntity);//####
    _backgroundEntities->push_back(anEntity);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::addEntityToBackground

void ServiceViewerApp::addEntityToForeground(ServiceEntity * anEntity)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("anEntity = ", anEntity);//####
    _foregroundEntities->push_back(anEntity);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::addEntityToForeground

void ServiceViewerApp::addPortConnectionsToBackground(const MplusM::Utilities::PortVector & detectedPorts)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("detectedPorts = ", &detectedPorts);//####
    for (MplusM::Utilities::PortVector::const_iterator outer(detectedPorts.begin()); detectedPorts.end() != outer;
         ++outer)
    {
        if (_rememberedPorts.end() != _rememberedPorts.find(outer->_portName))
        {
            ConnectionDetails             details;
            MplusM::Common::ChannelVector inputs;
            MplusM::Common::ChannelVector outputs;
            
            details._outPortName = outer->_portName;
            MplusM::Utilities::GatherPortConnections(outer->_portName, inputs, outputs,
                                                     MplusM::Utilities::kInputAndOutputOutput, true);
            for (MplusM::Common::ChannelVector::const_iterator inner(outputs.begin()); outputs.end() != inner; ++inner)
            {
                if (_rememberedPorts.end() != _rememberedPorts.find(inner->_portName))
                {
                    details._inPortName = inner->_portName;
                    details._mode = inner->_portMode;
                    _connections.push_back(details);
                }
            }
        }
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::addPortConnectionsToBackground

void ServiceViewerApp::addPortsWithAssociatesToBackground(const MplusM::Utilities::PortVector & detectedPorts)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("detectedPorts = ", &detectedPorts);//####
    for (MplusM::Utilities::PortVector::const_iterator outer(detectedPorts.begin()); detectedPorts.end() != outer;
         ++outer)
    {
        if (_rememberedPorts.end() == _rememberedPorts.find(outer->_portName))
        {
            PortAndAssociates associates;
            
            if (MplusM::Utilities::GetAssociatedPorts(outer->_portName, associates._associates, STANDARD_WAIT_TIME,
                                                      true))
            {
                if (associates._associates._primary)
                {
                    yarp::os::ConstString caption(outer->_portIpAddress + ":" + outer->_portPortNumber);
                    
                    associates._name = outer->_portName;
                    _associatedPorts.insert(AssociatesMap::value_type(caption.c_str(), associates));
                    _rememberedPorts.insert(outer->_portName);
                    for (MplusM::Common::StringVector::const_iterator inner(associates._associates._inputs.begin());
                         associates._associates._inputs.end() != inner; ++inner)
                    {
                        _rememberedPorts.insert(*inner);
                    }
                    for (MplusM::Common::StringVector::const_iterator inner(associates._associates._outputs.begin());
                         associates._associates._outputs.end() != inner; ++inner)
                    {
                        _rememberedPorts.insert(*inner);
                    }
                }
            }
        }
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::addPortsWithAssociatesToBackground

void ServiceViewerApp::addRegularPortEntitiesToBackground(const MplusM::Utilities::PortVector & detectedPorts)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("detectedPorts = ", &detectedPorts);//####
    for (MplusM::Utilities::PortVector::const_iterator walker(detectedPorts.begin()); detectedPorts.end() != walker;
         ++walker)
    {
        if (_rememberedPorts.end() == _rememberedPorts.find(walker->_portName))
        {
            yarp::os::ConstString caption(walker->_portIpAddress + ":" + walker->_portPortNumber);
            NameAndDirection      info;
            
            _rememberedPorts.insert(walker->_portName);
            info._name = walker->_portName;
            info._direction = determinePortDirection(walker->_portName);
            _standalonePorts.insert(PortMap::value_type(caption.c_str(), info));
        }
    }
    OD_LOG_OBJEXIT();//####
} // addRegularPortEntitiesToBackground

void ServiceViewerApp::addServicesToBackground(const MplusM::Common::StringVector & services)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("services = ", &services);//####
    for (MplusM::Common::StringVector::const_iterator outer(services.begin()); services.end() != outer; ++outer)
    {
        if (_detectedServices.end() == _detectedServices.find(outer->c_str()))
        {
            MplusM::Utilities::ServiceDescriptor descriptor;
            
            if (MplusM::Utilities::GetNameAndDescriptionForService(*outer, descriptor, STANDARD_WAIT_TIME))
            {
                _detectedServices.insert(ServiceMap::value_type(outer->c_str(), descriptor));
                _rememberedPorts.insert(descriptor._channelName);
                for (MplusM::Common::StringVector::const_iterator inner(descriptor._inputChannels.begin());
                     descriptor._inputChannels.end() != inner; ++inner)
                {
                    _rememberedPorts.insert(*inner);
                }
                for (MplusM::Common::StringVector::const_iterator inner(descriptor._outputChannels.begin());
                     descriptor._outputChannels.end() != inner; ++inner)
                {
                    _rememberedPorts.insert(*inner);
                }
            }
        }
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::addServicesToBackground

void ServiceViewerApp::clearDragState(void)
{
    OD_LOG_OBJENTER();//####
    if (_firstAddPort)
    {
        // Process connection add requests.
        ServiceEntity * firstEntity = findForegroundEntityForPort(_firstAddPort);
        
        if (firstEntity)
        {
            firstEntity->clearConnectMarker();
        }
        _firstAddPort = NULL;
    }
    _dragActive = false;
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::clearDragState

void ServiceViewerApp::dragEvent(ofDragInfo dragInfo)
{
    OD_LOG_OBJENTER();//####
    inherited::dragEvent(dragInfo);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::dragEvent

void ServiceViewerApp::draw(void)
{
//    OD_LOG_OBJENTER();//####
    ofBackgroundGradient(ofColor::white, ofColor::gray);
    if (_networkAvailable)
    {
        _foregroundLock.lock();
        for (EntityList::const_iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
        {
            ServiceEntity * anEntity = *it;
            
            if (anEntity && (! anEntity->isSelected()))
            {
                anEntity->draw();
            }
        }
        for (EntityList::const_iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
        {
            ServiceEntity * anEntity = *it;
            
            if (anEntity && anEntity->isSelected())
            {
                anEntity->draw();
            }
        }
        if (_dragActive)
        {
            if (_firstAddPort)
            {
                _firstAddPort->drawDragLine(_dragXpos, _dragYpos, _addingUDPConnection);
            }
            else
            {
                _dragActive = false;
            }
        }
        _foregroundLock.unlock();
    }
    else
    {
        string      title = "The YARP network is not running";
        ofRectangle bbox = ofBitmapStringGetBoundingBox(title, 0, 0);
        ofMesh &    mesh = ofBitmapStringGetMesh(title, (ofGetWidth() - bbox.width) / 2,
                                                 (ofGetHeight() - bbox.height) / 2);
        
        ofEnableAlphaBlending();
        ofSetColor(ofColor::black);
		ofBitmapStringGetTextureRef().bind();
        mesh.draw();
		ofBitmapStringGetTextureRef().unbind();
        ofDisableAlphaBlending();
    }
//    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::draw

void ServiceViewerApp::exit(void)
{
    OD_LOG_OBJENTER();//####
    if (_scanner && _scanner->isThreadRunning())
    {
        _scanner->stopThread();
    }
    destroyDirectionTestPorts();
    inherited::exit();
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::exit

PortEntry * ServiceViewerApp::findBackgroundPort(string name)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_S1("name = ", name.c_str());//####
    PortEntry *                  result;
    PortEntryMap::const_iterator match(_backgroundPorts->find(name));
    
    if (_backgroundPorts->end() == match)
    {
        result = NULL;
    }
    else
    {
        result = match->second;
    }
    OD_LOG_OBJEXIT_P(result);//####
    return result;
} // ServiceViewerApp::findBackgroundPort

ServiceEntity * ServiceViewerApp::findForegroundEntity(string name)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_S1("name = ", name.c_str());//####
    ServiceEntity * result = NULL;
    
    for (EntityList::const_iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
    {
        ServiceEntity * anEntity = *it;
        
        if (anEntity && (name == anEntity->getName()))
        {
            result = anEntity;
            break;
        }
        
    }
    OD_LOG_OBJEXIT_P(result);//####
    return result;
} // ServiceViewerApp::findForegroundEntity

ServiceEntity * ServiceViewerApp::findForegroundEntityForPort(string name)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_S1("name = ", name.c_str());//####
    PortEntryMap::const_iterator match(_foregroundPorts->find(name));
    ServiceEntity *              result = NULL;
    
    if (_foregroundPorts->end() != match)
    {
        for (EntityList::const_iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
        {
            ServiceEntity * anEntity = *it;
            
            if (anEntity && anEntity->hasPort(match->second))
            {
                result = anEntity;
                break;
            }
            
        }
    }
    OD_LOG_OBJEXIT_P(result);//####
    return result;
} // ServiceViewerApp::findForegroundEntityForPort

ServiceEntity * ServiceViewerApp::findForegroundEntityForPort(const PortEntry * aPort)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("aPort = ", aPort);//####
    ServiceEntity *      result = NULL;
    
    for (EntityList::const_iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
    {
        ServiceEntity * anEntity = *it;
        
        if (anEntity && anEntity->hasPort(aPort))
        {
            result = anEntity;
            break;
        }
        
    }
    OD_LOG_OBJEXIT_P(result);//####
    return result;
} // ServiceViewerApp::findForegroundEntityForPort

PortEntry * ServiceViewerApp::findForegroundPort(string name)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_S1("name = ", name.c_str());//####
    PortEntry *                  result;
    PortEntryMap::const_iterator match(_foregroundPorts->find(name));
    
    if (_foregroundPorts->end() == match)
    {
        result = NULL;
    }
    else
    {
        result = match->second;
    }
    OD_LOG_OBJEXIT_P(result);//####
    return result;
} // ServiceViewerApp::findForegroundPort

void ServiceViewerApp::forgetPort(PortEntry * aPort)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("aPort = ", aPort);//####
    if (aPort)
    {
        PortEntryMap::iterator match(_foregroundPorts->find(aPort->getName()));
        
        if (_foregroundPorts->end() != match)
        {
            _foregroundPorts->erase(match);
        }
        match = _backgroundPorts->find(aPort->getName());
        if (_backgroundPorts->end() != match)
        {
            _backgroundPorts->erase(match);
        }
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::forgetPort

void ServiceViewerApp::gatherEntitiesInBackground(void)
{
    OD_LOG_OBJENTER();//####
    MplusM::Utilities::PortVector detectedPorts;
    MplusM::Common::StringVector  services;
    
    // Mark the our utility ports as known.
    _rememberedPorts.insert(lInputOnlyPortName);
    _rememberedPorts.insert(lOutputOnlyPortName);
    MplusM::Utilities::GetDetectedPortList(detectedPorts);
    MplusM::Utilities::GetServiceNames(services, true);
    // Record the services to be displayed.
    addServicesToBackground(services);
    // Record the ports that have associates.
    if (MplusM::Utilities::CheckForRegistryService(detectedPorts))
    {
        addPortsWithAssociatesToBackground(detectedPorts);
    }
    // Record the ports that are standalone.
    addRegularPortEntitiesToBackground(detectedPorts);
    // Record the port connections.
    addPortConnectionsToBackground(detectedPorts);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::gatherEntitiesInBackground

void ServiceViewerApp::gotMessage(ofMessage msg)
{
    OD_LOG_OBJENTER();//####
    inherited::gotMessage(msg);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::gotMessage

void ServiceViewerApp::keyPressed(int key)
{
//    OD_LOG_OBJENTER();//####
//    OD_LOG_L1("key = ", key);//####
    if (OF_KEY_ALT == (key & OF_KEY_ALT))
    {
        _altActive = _addIsActive = true;
    }
    if (OF_KEY_COMMAND == (key & OF_KEY_COMMAND))
    {
        _commandActive = _removeIsActive = true;
    }
    if (OF_KEY_CONTROL == (key & OF_KEY_CONTROL))
    {
        _controlActive = true;
    }
    if (OF_KEY_SHIFT == (key & OF_KEY_SHIFT))
    {
        _shiftActive = true;
    }
    inherited::keyPressed(key);
//    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::keyPressed

void ServiceViewerApp::keyReleased(int key)
{
//    OD_LOG_OBJENTER();//####
//    OD_LOG_L1("key = ", key);//####
    if (OF_KEY_ALT == (key & OF_KEY_ALT))
    {
        _altActive = false;
    }
    if (OF_KEY_COMMAND == (key & OF_KEY_COMMAND))
    {
        _commandActive = false;
    }
    if (OF_KEY_CONTROL == (key & OF_KEY_CONTROL))
    {
        _controlActive = false;
    }
    if (OF_KEY_SHIFT == (key & OF_KEY_SHIFT))
    {
        _shiftActive = false;
    }
    inherited::keyReleased(key);
//    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::keyReleased

void ServiceViewerApp::mouseDragged(int x,
                                    int y,
                                    int button)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_L3("x = ", x, "y = ", y, "button = ", button);//####
    inherited::mouseDragged(x, y, button);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::mouseDragged

void ServiceViewerApp::mouseMoved(int x,
                                  int y)
{
//    OD_LOG_OBJENTER();//####
//    OD_LOG_L2("x = ", x, "y = ", y);//####
    inherited::mouseMoved(x, y);
//    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::mouseMoved

void ServiceViewerApp::mousePressed(int x,
                                    int y,
                                    int button)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_L3("x = ", x, "y = ", y, "button = ", button);//####
    inherited::mousePressed(x, y, button);
    reportPortEntryClicked(NULL);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::mousePressed

void ServiceViewerApp::mouseReleased(int x,
                                     int y,
                                     int button)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_L3("x = ", x, "y = ", y, "button = ", button);//####
    inherited::mouseReleased(x, y, button);
    clearDragState();
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::mouseReleased

void ServiceViewerApp::moveEntityToEndOfForegroundList(ServiceEntity * anEntity)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("anEntity = ", anEntity);//####
    if (anEntity->isSelected())
    {
        EntityList::iterator it(_foregroundEntities->begin());
        
        for ( ; _foregroundEntities->end() != it; ++it)
        {
            if (anEntity == *it)
            {
                _foregroundEntities->erase(it);
                addEntityToForeground(anEntity);
                break;
            }
            
        }
        _movementActive = false;
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::moveEntityToEndOfForegroundList

void ServiceViewerApp::rememberPortInBackground(PortEntry * aPort)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("aPort = ", aPort);//####
    if (aPort)
    {
        _backgroundPorts->insert(PortEntryMap::value_type(static_cast<string>(*aPort), aPort));
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::rememberPortInBackground

void ServiceViewerApp::reportConnectionDrag(const float xPos,
                                            const float yPos)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_D2("xPos = ", xPos, "yPos = ", yPos);//####
    if (_firstAddPort)
    {
        _dragActive = true;
        _dragXpos = xPos;
        _dragYpos = yPos;
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::reportConnectionDrag

void ServiceViewerApp::reportPortEntryClicked(PortEntry * aPort)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("aPort = ", aPort);//####
    if (aPort)
    {
        PortEntry::PortDirection direction = aPort->getDirection();
        PortEntry::PortUsage     usage = aPort->getUsage();
        
        if (_firstRemovePort)
        {
            // Process connection delete requests.
            ServiceEntity * firstEntity = findForegroundEntityForPort(_firstRemovePort);
            
            if (_firstRemovePort != aPort)
            {
                // Check if we can end here.
                if (PortEntry::kPortDirectionOutput != direction)
                {
                    ServiceEntity * secondEntity = findForegroundEntityForPort(aPort);
                    
                    if (firstEntity && secondEntity)
                    {
                        if (MplusM::Utilities::RemoveConnection(_firstRemovePort->getPortName().c_str(),
                                                                aPort->getPortName().c_str()))
                        {
                            _firstRemovePort->removeOutputConnection(aPort);
                            aPort->removeInputConnection(_firstRemovePort);
                        }
                    }
                }
            }
            if (firstEntity)
            {
                firstEntity->clearDisconnectMarker();
            }
            _firstRemovePort = NULL;
            _addIsActive = _removeIsActive = false;
        }
        else if (_firstAddPort)
        {
            // Process connection add requests.
            ServiceEntity * firstEntity = findForegroundEntityForPort(_firstAddPort);
            
            if (_firstAddPort != aPort)
            {
                // Check if we can end here.
                if (PortEntry::kPortDirectionOutput != direction)
                {
                    ServiceEntity * secondEntity = findForegroundEntityForPort(aPort);
                    
                    if (firstEntity && secondEntity)
                    {
                        if (MplusM::Utilities::AddConnection(_firstAddPort->getPortName().c_str(),
                                                             aPort->getPortName().c_str(), STANDARD_WAIT_TIME,
                                                             _addingUDPConnection))
                        {
                            MplusM::Common::ChannelMode mode = (_addingUDPConnection ?
                                                                MplusM::Common::kChannelModeUDP :
                                                                MplusM::Common::kChannelModeTCP);
                            
                            _firstAddPort->addOutputConnection(aPort, mode);
                            aPort->addInputConnection(_firstAddPort, mode);
                        }
                    }
                }
            }
            if (firstEntity)
            {
                firstEntity->clearConnectMarker();
            }
            _firstAddPort = NULL;
            _addIsActive = _removeIsActive = false;
        }
        else if (_commandActive)
        {
            // Check if we can start from here.
            if ((PortEntry::kPortDirectionInput != direction) && (PortEntry::kPortUsageClient != usage))
            {
                ServiceEntity * entity = findForegroundEntityForPort(aPort);
                
                if (entity)
                {
                    entity->setDisconnectMarker();
                    _firstRemovePort = aPort;
                }
            }
            _addIsActive = false;
            _removeIsActive = (NULL != _firstRemovePort);
        }
        else if (_altActive)
        {
            // Check if we can start from here.
            if ((PortEntry::kPortDirectionInput != direction) && (PortEntry::kPortUsageClient != usage))
            {
                ServiceEntity * entity = findForegroundEntityForPort(aPort);
                
                if (entity)
                {
                    entity->setConnectMarker();
                    _firstAddPort = aPort;
                    _addingUDPConnection = _shiftActive;
                }
            }
            _addIsActive = (NULL != _firstAddPort);
            _removeIsActive = false;
        }
        else
        {
            _addIsActive = _removeIsActive = false;
        }
    }
    else
    {
        // Operation being cleared.
        if (_commandActive || _removeIsActive)
        {
            // Process connection delete requests.
            if (_firstRemovePort)
            {
                ServiceEntity * entity = findForegroundEntityForPort(_firstRemovePort);
                
                if (entity)
                {
                    entity->clearDisconnectMarker();
                }
                _firstRemovePort = NULL;
            }
        }
        else if (_altActive || _addIsActive)
        {
            // Process connection add requests.
            if (_firstAddPort)
            {
                ServiceEntity * entity = findForegroundEntityForPort(_firstAddPort);
                
                if (entity)
                {
                    entity->clearConnectMarker();
                }
                _firstAddPort = NULL;
            }
        }
        _addIsActive = _removeIsActive = false;
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::reportPortEntryClicked

void ServiceViewerApp::setEntityPositions(void)
{
    OD_LOG_OBJENTER();//####
    bool  positionsNeedUpdate = false;
    float fullHeight = ofGetHeight();
    float fullWidth = ofGetWidth();
    float diagonal = sqrt((fullHeight * fullHeight) + (fullWidth * fullWidth));
    
#if defined(TEST_GRAPHICS_)
    for (EntityList::const_iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
    {
        ServiceEntity * anEntity = *it;

        if (anEntity)
        {
            ServiceEntity * olderVersion = findForegroundEntity(anEntity->getName());
            
            if (olderVersion)
            {
                ofRectangle oldShape(olderVersion->getShape());
                
                anEntity->setPosition(oldShape.getX(), oldShape.getY());
            }
            else
            {
                ofRectangle entityShape(anEntity->getShape());
                float       newX = ofRandom(fullWidth - entityShape.width);
                float       newY = ofRandom(fullHeight - entityShape.height);
                
                anEntity->setPosition(newX, newY);
            }
        }
    }
#else // ! defined(TEST_GRAPHICS_)
    ogdf::Graph           gg;
    ogdf::GraphAttributes ga(gg);
    
    ga.directed(true);
    for (EntityList::const_iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
    {
        ServiceEntity * anEntity = *it;
        
        if (anEntity)
        {
            float           newX;
            float           newY;
            ofRectangle     entityShape(anEntity->getShape());
            ogdf::node      aNode = gg.newNode();
            ServiceEntity * olderVersion = findForegroundEntity(anEntity->getName());
            
            ga.width(aNode) = entityShape.width;
            ga.height(aNode) = entityShape.height;
            anEntity->setNode(aNode);
            if (olderVersion)
            {
                ofRectangle oldShape(olderVersion->getShape());
                
                newX = oldShape.getX();
                newY = oldShape.getY();
            }
            else
            {
                newX = ofRandom(fullWidth - entityShape.width);
                newY = ofRandom(fullHeight - entityShape.height);
                positionsNeedUpdate = true;
            }
            ga.x(aNode) = newX;
            ga.y(aNode) = newY;
            anEntity->setPosition(newX, newY);
        }
    }
    if (positionsNeedUpdate)
    {
        // Set up the edges (connections)
        for (EntityList::const_iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
        {
            ServiceEntity * anEntity = *it;
            
            if (anEntity)
            {
                ogdf::node thisNode = anEntity->getNode();
                
                // Add edges between entities that are connected via their entries
                for (int ii = 0, mm = anEntity->getNumPorts(); mm > ii; ++ii)
                {
                    PortEntry * aPort = anEntity->getPort(ii);
                    
                    if (aPort)
                    {
                        const PortEntry::Connections & outputs(aPort->getOutputConnections());
                        
                        for (int jj = 0, nn = outputs.size(); nn > jj; ++jj)
                        {
                            PortEntry * otherPort = outputs[jj]._otherPort;
                            
                            if (otherPort)
                            {
                                PortPanel * otherParent = otherPort->getParent();
                                
                                if (otherParent)
                                {
                                    ServiceEntity & otherEntity = otherParent->getContainer();
                                    ogdf::node      otherNode = otherEntity.getNode();
                                    ogdf::edge      ee = gg.newEdge(thisNode, otherNode);
                                    
                                }
                            }
                        }
                    }
                }
            }
        }
        // Apply an energy-based layout
        ogdf::FMMMLayout fmmm;
        
        fmmm.useHighLevelOptions(true);
        fmmm.newInitialPlacement(false);//true);
        fmmm.qualityVersusSpeed(ogdf::FMMMLayout::qvsGorgeousAndEfficient);
        fmmm.allowedPositions(ogdf::FMMMLayout::apAll);
        fmmm.initialPlacementMult(ogdf::FMMMLayout::ipmAdvanced);
        fmmm.initialPlacementForces(ogdf::FMMMLayout::ipfKeepPositions);
        fmmm.repForcesStrength(2.0);
        fmmm.call(ga);
        for (EntityList::const_iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
        {
            ServiceEntity * anEntity = *it;
            
            if (anEntity)
            {
                ogdf::node aNode = anEntity->getNode();
                
                if (aNode)
                {
                    anEntity->setPosition(ga.x(aNode), ga.y(aNode));
                }
            }
        }
    }
#endif // ! defined(TEST_GRAPHICS_)
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::setEntityPositions

void ServiceViewerApp::setup(void)
{
    OD_LOG_OBJENTER();//####
    ofSetWindowTitle("Service Viewer");
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
#if CheckNetworkWorks_
    if (yarp::os::Network::checkNetwork())
#endif // CheckNetworkWorks_
    {
        _networkAvailable = true;
        createDirectionTestPorts();
        _scanner->startThread(false, false); // non-blocking, non-verbose
        _scanner->enableScan();
    }
#if CheckNetworkWorks_
    else
    {
        OD_LOG("! (yarp::os::Network::checkNetwork())");//####
        cerr << "YARP network not running." << endl;
        _networkAvailable = false;
    }
#endif // CheckNetworkWorks_
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::setup

void ServiceViewerApp::swapBackgroundAndForeground(void)
{
    OD_LOG_OBJENTER();//####
    if ((! _firstAddPort) && (! _firstRemovePort) && (! _movementActive))
    {
        _foregroundLock.lock();
        swap(_backgroundPorts, _foregroundPorts);
        swap(_backgroundEntities, _foregroundEntities);
        _foregroundLock.unlock();
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::swapBackgroundAndForeground

void ServiceViewerApp::update(void)
{
//    OD_LOG_OBJENTER();//####
    inherited::update();
    if (_scanner)
    {
        for (bool locked = _scanner->lock(); ! locked; )
        {
            sleep(SHORT_SLEEP);
        }
        bool scanDataReady = _scanner->scanComplete();
        
        _scanner->unlock();
        if (scanDataReady)
        {
            // Convert the detected services into entities in the background list.
            for (ServiceMap::const_iterator outer(_detectedServices.begin()); _detectedServices.end() != outer;
                 ++outer)
            {
                MplusM::Utilities::ServiceDescriptor descriptor(outer->second);
                ServiceEntity *                      anEntity = new ServiceEntity(PortPanel::kEntityKindService,
                                                                                  descriptor._description.c_str(),
                                                                                  *this);
                
                anEntity->setup(descriptor._canonicalName);
                PortEntry * aPort = anEntity->addPort(descriptor._channelName, PortEntry::kPortUsageService,
                                                      PortEntry::kPortDirectionInput);
                
                if (aPort)
                {
                    rememberPortInBackground(aPort);
                }
                for (MplusM::Common::StringVector::const_iterator inner(descriptor._inputChannels.begin());
                     descriptor._inputChannels.end() != inner; ++inner)
                {
                    aPort = anEntity->addPort(*inner, PortEntry::kPortUsageService, PortEntry::kPortDirectionInput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                }
                for (MplusM::Common::StringVector::const_iterator inner(descriptor._outputChannels.begin());
                     descriptor._outputChannels.end() != inner; ++inner)
                {
                    aPort = anEntity->addPort(*inner, PortEntry::kPortUsageService, PortEntry::kPortDirectionOutput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                }
                addEntityToBackground(anEntity);
            }
            // Convert the detected ports with associates into entities in the background list.
            for (AssociatesMap::const_iterator outer(_associatedPorts.begin()); _associatedPorts.end() != outer;
                 ++outer)
            {
                PortEntry *     aPort;
                ServiceEntity * anEntity = new ServiceEntity(PortPanel::kEntityKindClientOrAdapter, "", *this);
                
                anEntity->setup(outer->first.c_str());
                for (MplusM::Common::StringVector::const_iterator inner(outer->second._associates._inputs.begin());
                     outer->second._associates._inputs.end() != inner; ++inner)
                {
                    aPort = anEntity->addPort(*inner, PortEntry::kPortUsageOther,
                                              PortEntry::kPortDirectionInput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                }
                for (MplusM::Common::StringVector::const_iterator inner(outer->second._associates._outputs.begin());
                     outer->second._associates._outputs.end() != inner; ++inner)
                {
                    aPort = anEntity->addPort(*inner, PortEntry::kPortUsageOther,
                                              PortEntry::kPortDirectionOutput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                }
                aPort = anEntity->addPort(outer->second._name, PortEntry::kPortUsageClient,
                                          PortEntry::kPortDirectionInputOutput);
                if (aPort)
                {
                    rememberPortInBackground(aPort);
                }
                addEntityToBackground(anEntity);
            }
            // Convert the detected standalone ports into entities in the background list.
            for (PortMap::const_iterator walker(_standalonePorts.begin()); _standalonePorts.end() != walker; ++walker)
            {
                ServiceEntity *      anEntity = new ServiceEntity(PortPanel::kEntityKindOther, "", *this);
                PortEntry::PortUsage usage;
                
                anEntity->setup(walker->first.c_str());
                switch (MplusM::Utilities::GetPortKind(walker->second._name))
                {
                    case MplusM::Utilities::kPortKindClient:
                        usage = PortEntry::kPortUsageClient;
                        break;
                        
                    case MplusM::Utilities::kPortKindService:
                    case MplusM::Utilities::kPortKindServiceRegistry:
                        usage = PortEntry::kPortUsageService;
                        break;
                        
                    default:
                        usage = PortEntry::kPortUsageOther;
                        break;
                        
                }
                PortEntry * aPort = anEntity->addPort(walker->second._name, usage, walker->second._direction);
                
                if (aPort)
                {
                    rememberPortInBackground(aPort);
                }
                addEntityToBackground(anEntity);
            }
            // Convert the detected connections into connections in the background list.
            for (ConnectionList::const_iterator walker(_connections.begin()); _connections.end() != walker; ++walker)
            {
                PortEntry * thisPort = findBackgroundPort(walker->_outPortName);
                PortEntry * otherPort = findBackgroundPort(walker->_inPortName);
                
                if (thisPort && otherPort)
                {
                    thisPort->addOutputConnection(otherPort, walker->_mode);
                    otherPort->addInputConnection(thisPort, walker->_mode);
                }
            }
            setEntityPositions();
            swapBackgroundAndForeground();
            // Clear out old data:
            for (EntityList::const_iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
            {
                ServiceEntity * anEntity = *it;
                
                if (anEntity)
                {
                    delete anEntity;
                }
            }
            _backgroundEntities->clear();
            // Note that the ports will have been deleted by the deletion of the entities.
            _backgroundPorts->clear();
            _detectedServices.clear();
            _rememberedPorts.clear();
            // Ignore our test ports!
            _associatedPorts.clear();
            _standalonePorts.clear();
            _connections.clear();
            _scanner->enableScan();
        }
    }
//    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::update

void ServiceViewerApp::windowResized(int w,
                                     int h)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_L2("w = ", w, "h = ", h);//####
    inherited::windowResized(w, h);
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::windowResized

#if defined(__APPLE__)
# pragma mark Accessors
#endif // defined(__APPLE__)

ofColor ServiceViewerApp::getMarkerColor(void)
{
    return ofColor::yellow;
} // ServiceViewerApp::getMarkerColor

ofColor ServiceViewerApp::getNewConnectionColor(void)
{
    return ofColor::gold;
} // ServiceViewerApp::getNewConnectionColor

float ServiceViewerApp::getNormalConnectionWidth(void)
{
    return kNormalConnectionWidth;
} // ServiceViewerApp::getNormalConnectionWidth

ofColor ServiceViewerApp::getOtherConnectionColor(void)
{
    return ofColor::orange;
} // ServiceViewerApp::getOtherConnectionColor

float ServiceViewerApp::getServiceConnectionWidth(void)
{
    return kServiceConnectionWidth;
} // ServiceViewerApp::getServiceConnectionWidth

ofColor ServiceViewerApp::getTcpConnectionColor(void)
{
    return ofColor::teal;
} // ServiceViewerApp::getTcpConnectionColor

ofColor ServiceViewerApp::getUdpConnectionColor(void)
{
    return ofColor::purple;
} // ServiceViewerApp::getUdpConnectionColor

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
