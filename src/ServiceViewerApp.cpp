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
#include "ServiceEntity.h"

//#include "ODEnableLogging.h"
#include "ODLogging.h"

#include "ofAppRunner.h"
#include "ofBitmapFont.h"
#include "ofGraphics.h"
#include "ofMesh.h"

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

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

/*! @brief Create the resources needed to determine port directions. */
static void createDirectionTestPorts(void)
{
    OD_LOG_ENTER();//####
    yarp::os::ConstString inName(MplusM::Common::GetRandomChannelName("/checkdirection/channel_"));
    yarp::os::ConstString outName(MplusM::Common::GetRandomChannelName("/checkdirection/channel_"));
    
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
            if (lInputOnlyPort->openWithRetries(inName) && lOutputOnlyPort->openWithRetries(outName))
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
#if defined(MpM_DO_EXPLICIT_CLOSE)
        lInputOnlyPort->close();
#endif // defined(MpM_DO_EXPLICIT_CLOSE)
        MplusM::Common::AdapterChannel::RelinquishChannel(lInputOnlyPort);
    }
    if (lOutputOnlyPort)
    {
#if defined(MpM_DO_EXPLICIT_CLOSE)
        lOutputOnlyPort->close();
#endif // defined(MpM_DO_EXPLICIT_CLOSE)
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
        
        if (MplusM::Common::NetworkConnectWithRetries(lOutputOnlyPort->getName(), portName))
        {
            canDoInput = true;
            if (! MplusM::Common::NetworkDisconnectWithRetries(lOutputOnlyPort->getName(), portName))
            {
                OD_LOG("(! MplusM::Common::NetworkDisconnectWithRetries(lOutputOnlyPort->getName(), portName))");//####
            }
        }
        if (MplusM::Common::NetworkConnectWithRetries(portName, lInputOnlyPort->getName()))
        {
            canDoOutput = true;
            if (! MplusM::Common::NetworkDisconnectWithRetries(portName, lInputOnlyPort->getName()))
            {
                OD_LOG("(! MplusM::Common::NetworkDisconnectWithRetries(portName, lInputOnlyPort->getName()))");//####
            }
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
    OD_LOG_EXIT();//####
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
            _firstAddPort(NULL), _firstRemovePort(NULL), _addingUDPConnection(false), _addIsActive(false),
            _altActive(false), _commandActive(false), _controlActive(false), _dragActive(false),
            _movementActive(false), _networkAvailable(false), _registryAvailable(false), _removeIsActive(false),
            _shiftActive(false)
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
    for (size_t ii = 0, mm = detectedPorts.size(); mm > ii; ++ii)
    {
        const MplusM::Utilities::PortDescriptor & aDescriptor = detectedPorts[ii];
        MplusM::Common::ChannelVector             inputs;
        MplusM::Common::ChannelVector             outputs;
        PortEntry *                               thisPort = findBackgroundPort(aDescriptor._portName);
        
        if (thisPort)
        {
            MplusM::Utilities::GatherPortConnections(aDescriptor._portName, inputs, outputs,
                                                     MplusM::Utilities::kInputAndOutputOutput, true);
            for (int jj = 0, mm = outputs.size(); mm > jj; ++jj)
            {
                MplusM::Common::ChannelDescription aConnection = outputs[jj];
                PortEntry *                        otherPort = findBackgroundPort(aConnection._portName);
                
                if (otherPort)
                {
                    thisPort->addOutputConnection(otherPort, aConnection._portMode);
                    otherPort->addInputConnection(thisPort, aConnection._portMode);
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
    for (int ii = 0, mm = detectedPorts.size(); mm > ii; ++ii)
    {
        const MplusM::Utilities::PortDescriptor & aDescriptor = detectedPorts[ii];
        yarp::os::ConstString                     aPort(aDescriptor._portName);
        
        
        if (! findBackgroundPort(aDescriptor._portName.c_str()))
        {
            MplusM::Common::StringVector inputs;
            MplusM::Common::StringVector outputs;
            bool                         isPrimary;
            
            if (MplusM::Utilities::GetAssociatedPorts(aDescriptor._portName, inputs, outputs, isPrimary, true))
            {
                if (isPrimary)
                {
                    PortEntry *     aPort;
                    ServiceEntity * anEntity = new ServiceEntity(PortPanel::kEntityKindClientOrAdapter, "", *this);
                    
                    anEntity->setup((aDescriptor._portIpAddress + ":" + aDescriptor._portPortNumber).c_str());
                    for (int jj = 0, nn = inputs.size(); nn > jj; ++jj)
                    {
                        aPort = anEntity->addPort(inputs[jj], PortEntry::kPortUsageOther,
                                                  PortEntry::kPortDirectionInput);
                        if (aPort)
                        {
                            rememberPortInBackground(aPort);
                        }
                    }
                    for (int jj = 0, nn = outputs.size(); nn > jj; ++jj)
                    {
                        aPort = anEntity->addPort(outputs[jj], PortEntry::kPortUsageOther,
                                                  PortEntry::kPortDirectionOutput);
                        if (aPort)
                        {
                            rememberPortInBackground(aPort);
                        }
                    }
                    aPort = anEntity->addPort(aDescriptor._portName, PortEntry::kPortUsageClient,
                                              PortEntry::kPortDirectionInputOutput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                    addEntityToBackground(anEntity);
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
    createDirectionTestPorts();
    for (size_t ii = 0, mm = detectedPorts.size(); mm > ii; ++ii)
    {
        const MplusM::Utilities::PortDescriptor & aDescriptor = detectedPorts[ii];
        
        if (! findBackgroundPort(aDescriptor._portName.c_str()))
        {
            ServiceEntity * anEntity = new ServiceEntity(PortPanel::kEntityKindOther, "", *this);
            
            anEntity->setup((aDescriptor._portIpAddress + ":" + aDescriptor._portPortNumber).c_str());
            PortEntry * aPort = anEntity->addPort(aDescriptor._portName, PortEntry::kPortUsageOther,
                                                  determinePortDirection(aDescriptor._portName));
            
            if (aPort)
            {
                rememberPortInBackground(aPort);
            }
            addEntityToBackground(anEntity);
        }
    }
    destroyDirectionTestPorts();
    OD_LOG_OBJEXIT();//####
} // addRegularPortEntitiesToBackground

void ServiceViewerApp::addServicesToBackground(const MplusM::Common::StringVector & services)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_P1("services = ", &services);//####
    for (size_t ii = 0, mm = services.size(); mm > ii; ++ii)
    {
        yarp::os::ConstString aService = services[ii];
        
        if (! findBackgroundEntity(aService.c_str()))
        {
            MplusM::Utilities::ServiceDescriptor descriptor;
            
            if (MplusM::Utilities::GetNameAndDescriptionForService(aService, descriptor))
            {
                ServiceEntity * anEntity = new ServiceEntity(PortPanel::kEntityKindService,
                                                             descriptor._description.c_str(), *this);
                
                anEntity->setup(descriptor._canonicalName);
                PortEntry * aPort = anEntity->addPort(aService, PortEntry::kPortUsageService,
                                                      PortEntry::kPortDirectionInput);
                
                if (aPort)
                {
                    rememberPortInBackground(aPort);
                }
                for (int jj = 0, nn = descriptor._inputChannels.size(); nn > jj; ++jj)
                {
                    aPort = anEntity->addPort(descriptor._inputChannels[jj], PortEntry::kPortUsageService,
                                              PortEntry::kPortDirectionInput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                }
                for (int jj = 0, nn = descriptor._outputChannels.size(); nn > jj; ++jj)
                {
                    aPort = anEntity->addPort(descriptor._outputChannels[jj], PortEntry::kPortUsageService,
                                              PortEntry::kPortDirectionOutput);
                    if (aPort)
                    {
                        rememberPortInBackground(aPort);
                    }
                }
                addEntityToBackground(anEntity);
            }
        }
    }
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::addServicesToBackground

void ServiceViewerApp::clearDragState(void)
{
    OD_LOG_OBJENTER();//####
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
        for (EntityList::iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
        {
            ServiceEntity * anEntity = *it;
            
            if (anEntity && (! anEntity->isSelected()))
            {
                anEntity->draw();
            }
        }
        for (EntityList::iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
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
    inherited::exit();
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::exit

ServiceEntity * ServiceViewerApp::findBackgroundEntity(string name)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_S1("name = ", name.c_str());//####
    ServiceEntity * result = NULL;
    
    for (EntityList::iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
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
} // ServiceViewerApp::findBackgroundEntity

PortEntry * ServiceViewerApp::findBackgroundPort(string name)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_S1("name = ", name.c_str());//####
    PortEntry *       result;
    PortMap::iterator match(_backgroundPorts->find(name));
    
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
    
    for (EntityList::iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
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
    PortMap::iterator match(_foregroundPorts->find(name));
    ServiceEntity *   result = NULL;
    
    if (_foregroundPorts->end() != match)
    {
        for (EntityList::iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
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
    
    for (EntityList::iterator it(_foregroundEntities->begin()); _foregroundEntities->end() != it; ++it)
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
    PortEntry *       result;
    PortMap::iterator match(_foregroundPorts->find(name));
    
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
        PortMap::iterator match(_foregroundPorts->find(aPort->getName()));
        
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
    MplusM::Common::StringVector services;
    
    MplusM::Utilities::GetServiceNames(services, true);
    _backgroundEntities->clear();
    _backgroundPorts->clear();
    addServicesToBackground(services);
    MplusM::Utilities::PortVector detectedPorts;
    
    MplusM::Utilities::GetDetectedPortList(detectedPorts);
    // Identify and add ports that have associated ports, as they are adapters
    addPortsWithAssociatesToBackground(detectedPorts);
    // Add regular YARP ports as distinct entities
    addRegularPortEntitiesToBackground(detectedPorts);
    // Add the connections
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
        _backgroundPorts->insert(PortMap::value_type(static_cast<string>(*aPort), aPort));
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
                                                             aPort->getPortName().c_str(), _addingUDPConnection))
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

void ServiceViewerApp::setInitialEntityPositions(void)
{
    OD_LOG_OBJENTER();//####
//#if defined(TEST_GRAPHICS_)
    float fullHeight = ofGetHeight();
    float fullWidth = ofGetWidth();
    
    for (EntityList::iterator it(_backgroundEntities->begin()); _backgroundEntities->end() != it; ++it)
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
//#else // ! defined(TEST_GRAPHICS_)
//#endif // ! defined(TEST_GRAPHICS_)
    OD_LOG_OBJEXIT();//####
} // ServiceViewerApp::setInitialEntityPositions

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
        gatherEntitiesInBackground();
        setInitialEntityPositions();
        swapBackgroundAndForeground();
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
    ofLogVerbose() << "updating";
    inherited::update();
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
