//--------------------------------------------------------------------------------------------------
//
//  File:       ServiceEntity.cpp
//
//  Project:    M+M
//
//  Contains:   The class definition for a moveable GUI element to represent a set of
//              ports.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by H Plus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms, with or
//              without modification, are permitted provided that the following conditions are met:
//                * Redistributions of source code must retain the above copyright notice, this list
//                  of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright notice, this
//                  list of conditions and the following disclaimer in the documentation and/or
//                  other materials provided with the distribution.
//                * Neither the name of the copyright holders nor the names of its contributors may
//                  be used to endorse or promote products derived from this software without
//                  specific prior written permission.
//
//              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//              EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//              OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//              SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//              INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//              TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//              BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//              CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//              ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//              DAMAGE.
//
//  Created:    2014-05-09
//
//--------------------------------------------------------------------------------------------------

#include "ServiceEntity.h"
#include "ServiceViewerApp.h"
#include "Utilities.h"

//#include <odlEnable.h>
#include <odlInclude.h>

#include "ofGraphics.h"

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 
 @brief The class definition for a moveable GUI element to represent a set of ports. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The width and height of the marker displayed during movement. */
static const float kMarkerSide = 12;

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and Destructors
#endif // defined(__APPLE__)

ServiceEntity::ServiceEntity(const PortPanel::EntityKind kind,
                             const string &              behaviour,
                             const string &              description,
                             ServiceViewerApp &          owner) :
    inherited(), _panel(kind, behaviour, description, owner, *this), _node(NULL),
    _drawConnectMarker(false), _drawDisconnectMarker(false), _drawMoveMarker(false),
    _selected(false)
{
    ODL_ENTER(); //####
    ODL_S2s("behaviour = ", behaviour, "description = ", description); //####
    ODL_P1("owner = ", &owner); //####
    ODL_EXIT_P(this); //####
} // ServiceEntity::ServiceEntity

ServiceEntity::~ServiceEntity(void)
{
    ODL_OBJENTER(); //####
    ODL_OBJEXIT(); //####
} // ServiceEntity::~ServiceEntity

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

void
ServiceEntity::draw(void)
{
    ODL_OBJENTER(); //####
    ServiceViewerApp & owner = _panel.getOwner();
    
    _panel.draw();
    if (_drawConnectMarker)
    {
        ofPoint markerPos(_panel.getWidth() / 2, _panel.getHeader());
        
        ofSetColor(ServiceViewerApp::getMarkerColor());
        ofFill();
        ofCircle(markerPos + getShape().getTopLeft(), kMarkerSide / 2);
    }
    else if (_drawDisconnectMarker)
    {
        ofPoint markerPos(_panel.getWidth() / 2, _panel.getHeader());
        
        ofSetColor(ServiceViewerApp::getMarkerColor());
        ofNoFill();
        ofSetLineWidth(2);
        ofCircle(markerPos + getShape().getTopLeft(), kMarkerSide / 2);
    }
    else if (_drawMoveMarker)
    {
        ofPoint markerPos(_panel.getWidth() / 2, _panel.getHeader());
        
        ofSetColor(ServiceViewerApp::getMarkerColor());
        ofFill();
        markerPos += getShape().getTopLeft() - ofPoint(kMarkerSide / 2, kMarkerSide / 2);
        ofRect(markerPos, kMarkerSide, kMarkerSide);
    }
    for (int ii = 0, mm = _panel.getNumPorts(); mm > ii; ++ii)
    {
        PortEntry * anEntry = _panel.getPort(ii);
        
        if (anEntry)
        {
            ofPoint                        aCentre(anEntry->getCentre());
            const PortEntry::Connections & connex = anEntry->getOutputConnections();
            
            for (PortEntry::Connections::const_iterator walker(connex.begin());
                 connex.end() != walker; ++walker)
            {
                PortEntry::AnchorSide       anchorHere;
                PortEntry::AnchorSide       anchorThere;
                MplusM::Common::ChannelMode mode = walker->_connectionMode;
                PortEntry *                 otherEntry = walker->_otherPort;
                ofPoint                     otherCentre(otherEntry->getCentre());
                ofPoint                     fromHere;
                ofPoint                     toThere;
                
                // Check if the destination is above the source, in which case we
                // determine the anchors in the reverse order.
                if (aCentre.y < otherCentre.y)
                {
                    anchorHere = anEntry->calculateClosestAnchor(fromHere, true, false,
                                                                 otherCentre);
                    anchorThere = otherEntry->calculateClosestAnchor(toThere, false,
                                                     PortEntry::kAnchorBottomCentre == anchorHere,
                                                                     aCentre);
                }
                else
                {
                    anchorThere = otherEntry->calculateClosestAnchor(toThere, false, false,
                                                                     aCentre);
                    anchorHere = anEntry->calculateClosestAnchor(fromHere, true,
                                                     PortEntry::kAnchorBottomCentre == anchorThere,
                                                                 otherCentre);
                }
                if (otherEntry->isService())
                {
                    ofSetLineWidth(owner.getServiceConnectionWidth());
                }
                else if (otherEntry->isInputOutput())
                {
                    ofSetLineWidth(owner.getInputOutputConnectionWidth());
                }
                else
                {
                    ofSetLineWidth(owner.getNormalConnectionWidth());
                }
                switch (mode)
                {
                    case MplusM::Common::kChannelModeTCP :
                        ofSetColor(ServiceViewerApp::getTcpConnectionColor());
                        break;
                        
                    case MplusM::Common::kChannelModeUDP :
                        ofSetColor(ServiceViewerApp::getUdpConnectionColor());
                        break;
                        
                    default :
                        ofSetColor(ServiceViewerApp::getOtherConnectionColor());
                        break;
                        
                }
                DrawBezier(fromHere, toThere, aCentre, otherCentre);
                ofSetLineWidth(1);
                PortEntry::drawSourceAnchor(anchorHere, fromHere);
                PortEntry::drawTargetAnchor(anchorThere, toThere);
            }
        }
    }
    ODL_OBJEXIT(); //####
} // ServiceEntity::draw

void
ServiceEntity::handlePositionChange(void)
{
    ODL_OBJENTER(); //####
    // Here we re-adjust any lines attached to the entity.
    _panel.getOwner().movementStarted();
    _selected = _drawMoveMarker = true;
    ODL_OBJEXIT(); //####
} // ServiceEntity::handlePositionChange

bool
ServiceEntity::hasPort(const PortEntry * aPort)
{
    ODL_OBJENTER(); //####
    bool result = false;
    
    for (int ii = 0, mm = _panel.getNumPorts(); mm > ii; ++ii)
    {
        PortEntry * anEntry = _panel.getPort(ii);
        
        if (aPort == anEntry)
        {
            result = true;
            break;
        }
        
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // ServiceEntity::hasPort

void
ServiceEntity::positionChangeComplete(void)
{
    ODL_OBJENTER(); //####
    _panel.getOwner().moveEntityToEndOfForegroundList(this);
    _selected = _drawMoveMarker = false;
    // something to do??
    ODL_OBJEXIT(); //####
} // ServiceEntity::positionChangeComplete

void
ServiceEntity::setup(string      entityName,
                     const float xx,
                     const float yy)
{
    ODL_OBJENTER(); //####
    ODL_S1s("entityName = ", entityName); //####
    ODL_D2("xx = ", xx, "yy = ", yy); //####
    _panel.setup();
    _panel.setName(entityName);
    setPosition(xx, yy);
    _panel.setTracker(this);
    ODL_OBJEXIT(); //####
} // ServiceEntity::setup

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
