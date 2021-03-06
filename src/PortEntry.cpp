//--------------------------------------------------------------------------------------------------
//
//  File:       PortEntry.cpp
//
//  Project:    M+M
//
//  Contains:   The class definition for a GUI element to represent ports.
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
//  Created:    2014-05-12
//
//--------------------------------------------------------------------------------------------------

#include "ServiceEntity.h"
#include "ServiceViewerApp.h"
#include "Utilities.h"

//#include <odlEnable.h>
#include <odlInclude.h>

#include "ofGraphics.h"
#include "ofSystemUtils.h"

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 
 @brief The class definition for a GUI element to represent ports. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The horizontal and vertical length of the arrow 'arm'. */
static const float kArrowSize = 7;

/*! @brief The scale factor to apply to get the size of the target box. */
static const float kTargetBoxScale = 0.25;

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

/*! @brief Determine if a new point is closer to a reference point than the previous point.
 @param distanceSoFar On input, the closest distance so far and on output, the new closest distance.
 @param refPoint The point to measure distance from.
 @param testPoint The point being checked.
 @param bestSoFar On input, the current closest point and output, the new closest point.
 @returns @c true if the new point is closer than the previous closest point. */
static bool
calculateMinDistance(float &         distanceSoFar,
                     const ofPoint & refPoint,
                     const ofPoint & testPoint,
                     ofPoint &       bestSoFar)
{
    ODL_ENTER(); //####
    ODL_P4("distanceSoFar = ", &distanceSoFar, "refPoint = ", &refPoint, "testPoint = ", //####
              &testPoint, "bestSoFar = ", &bestSoFar); //####
    bool  result;
    float newDistance = refPoint.distance(testPoint);
    
    if (newDistance < distanceSoFar)
    {
        distanceSoFar = newDistance;
        bestSoFar = testPoint;
        result = true;
    }
    else
    {
        result = false;
    }
    ODL_EXIT_B(result); //####
    return result;
} // calculateMinDistance

/*! @brief Determine if a new point is closer to a reference point than the previous point.
 @param distanceSoFar On input, the closest distance so far and on output, the new closest distance.
 @param refPoint The point to measure distance from.
 @param testX The horizontal coordinate for the point being checked.
 @param testY The vertical coordinate for the point being checked.
 @param bestSoFar On input, the current closest point and output, the new closest point.
 @returns @c true if the new point is closer than the previous closest point. */
inline static bool
calculateMinDistance(float &         distanceSoFar,
                     const ofPoint & refPoint,
                     const float     testX,
                     const float     testY,
                     ofPoint &       bestSoFar)
{
    return calculateMinDistance(distanceSoFar, refPoint, ofPoint(testX, testY), bestSoFar);
} // calculateMinDistance

/*! @brief Determine the anchor point that is the minimum distance from a given point.
 @param newCentre The synthesized centre for the target point.
 @param disallowBottom @c true if the anchor cannot be bottom-centre.
 @param targetPoint The target point.
 @param refCentre The reference point.
 @returns The side to which the anchor is attached. */
static PortEntry::AnchorSide
calculateAnchorForPoint(ofPoint &       newCentre,
                        const bool      disallowBottom,
                        const ofPoint & targetPoint,
                        const ofPoint & refCentre)
{
    ODL_ENTER(); //####
    ODL_P3("newCentre = ", &newCentre, "targetPoint = ", &targetPoint, //####
              "refCentre = ", &refCentre); //####
    PortEntry::AnchorSide anchor = PortEntry::kAnchorUnknown;
    float                 boxSize = (refCentre.distance(targetPoint) * kTargetBoxScale);
    float                 soFar = 1e23;           // Ridiculously big, just in case.
    ofPoint               tempPoint;
    ofRectangle           box(targetPoint.x - (boxSize / 2), targetPoint.y - (boxSize / 2),
                              boxSize, boxSize);
    
    if (calculateMinDistance(soFar, refCentre, box.x, box.y + (boxSize / 2), tempPoint))
    {
        anchor = PortEntry::kAnchorLeft;
        newCentre = targetPoint + ofPoint(boxSize, 0);
    }
    if (calculateMinDistance(soFar, refCentre, box.x + boxSize, box.y + (boxSize / 2), tempPoint))
    {
        anchor = PortEntry::kAnchorRight;
        newCentre = targetPoint + ofPoint(-boxSize, 0);
    }
    if ((! disallowBottom) && calculateMinDistance(soFar, refCentre, box.x + (boxSize / 2),
                                                   box.y + boxSize, tempPoint))
    {
        anchor = PortEntry::kAnchorBottomCentre;
        newCentre = targetPoint + ofPoint(0, -boxSize);
    }
    if (calculateMinDistance(soFar, refCentre, box.x + (boxSize / 2), box.y, tempPoint))
    {
        anchor = PortEntry::kAnchorTopCentre;
        newCentre = targetPoint + ofPoint(0, boxSize);
    }
    ODL_EXIT_I(static_cast<int>(anchor)); //####
    return anchor;
} // calculateAnchorForPoint

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and Destructors
#endif // defined(__APPLE__)

PortEntry::PortEntry(IconlessPanel *     parent,
                     const string &      portProtocol,
                     const PortUsage     portKind,
                     const PortDirection direction) :
    inherited(parent), _portProtocol(portProtocol), _direction(direction), _usage(portKind),
    _isLastPort(true)
{
    ODL_ENTER(); //####
    ODL_P1("parent = ", parent); //####
    ODL_S1s("portProtocol = ", portProtocol.c_str()); //####
    ODL_L2("portKind = ", portKind, "direction = ", direction); //####
    ODL_EXIT_P(this); //####
} // PortEntry::PortEntry

PortEntry::~PortEntry(void)
{
    ODL_OBJENTER(); //####
    ODL_OBJEXIT(); //####
} // PortEntry::~PortEntry

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

/*! @brief Add an input connection to the port.
 @param other The port that is to be connected. */
void
PortEntry::addInputConnection(PortEntry *                 other,
                              MplusM::Common::ChannelMode mode)
{
    ODL_OBJENTER(); //####
    ODL_P1("other = ", other); //####
    if (other)
    {
        bool canAdd = true;
        
        for (Connections::const_iterator walker(_inputConnections.begin());
             _inputConnections.end() != walker; ++walker)
        {
            if (walker->_otherPort == other)
            {
                canAdd = false;
                break;
            }
            
        }
        if (canAdd)
        {
            PortConnection newConnection;
            
            newConnection._otherPort = other;
            newConnection._connectionMode = mode;
            _inputConnections.push_back(newConnection);
        }
    }
    ODL_OBJEXIT(); //####
} // PortEntry::addInputConnection

/*! @brief Add an output connection to the port.
 @param other The port that is to be connected. */
void
PortEntry::addOutputConnection(PortEntry *                 other,
                               MplusM::Common::ChannelMode mode)
{
    ODL_OBJENTER(); //####
    ODL_P1("other = ", other); //####
    if (other)
    {
        bool canAdd = true;
        
        for (Connections::const_iterator walker(_outputConnections.begin());
             _outputConnections.end() != walker; ++walker)
        {
            if (walker->_otherPort == other)
            {
                canAdd = false;
                break;
            }
            
        }
        if (canAdd)
        {
            PortConnection newConnection;
            
            newConnection._otherPort = other;
            newConnection._connectionMode = mode;
            _outputConnections.push_back(newConnection);
        }
    }
    ODL_OBJEXIT(); //####
} // PortEntry::addOutputConnection

PortEntry::AnchorSide
PortEntry::calculateClosestAnchor(ofPoint &       result,
                                  const bool      isSource,
                                  const bool      disallowBottom,
                                  const ofPoint & pp)
{
    ODL_OBJENTER(); //####
    ODL_P2("result = ", &result, "pp = ", &pp); //####
    ODL_B1("isSource = ", isSource); //####
    // Check each anchor point - the two side centres and optionally the bottom - to find the
    // shortest distance.
    AnchorSide  anchor = kAnchorUnknown;
    float       soFar = 1e23; // Ridiculously big, just in case.
    ofRectangle outer(getShape());
    
    if (calculateMinDistance(soFar, pp, outer.x, outer.y + (outer.height / 2), result))
    {
        anchor = kAnchorLeft;
        if (isSource)
        {
            // Adjust the anchor position if an output.
            result.x -= kArrowSize;
        }
    }
    if (calculateMinDistance(soFar, pp, outer.x + outer.width, outer.y + (outer.height / 2),
                             result))
    {
        anchor = kAnchorRight;
        if (isSource)
        {
            // Adjust the anchor position if an output.
            result.x += kArrowSize;
        }
    }
    if (_isLastPort && (! disallowBottom))
    {
        if (calculateMinDistance(soFar, pp, outer.x + (outer.width / 2), outer.y + outer.height,
                                 result))
        {
            anchor = kAnchorBottomCentre;
            if (isSource)
            {
                // Adjust the anchor position if an output.
                result.y += kArrowSize;
            }
        }
    }
    ODL_OBJEXIT_I(static_cast<int> (anchor)); //####
    return anchor;
} // PortEntry::calculateClosestAnchor

void
PortEntry::drawDragLine(const float xPos,
                        const float yPos,
                        const bool  isUDP)
{
    ODL_OBJENTER(); //####
    ODL_D2("xPos = ", xPos, "yPos = ", yPos); //####
    ODL_B1("isUDP = ", isUDP); //####
    PortPanel * theParent = getParent();
    
    if (! theParent->isPointInside(xPos, yPos))
    {
        AnchorSide anchorHere;
        AnchorSide anchorThere;
        ofPoint    aCentre(getCentre());
        ofPoint    toThere(xPos, yPos);
        ofPoint    fromHere;
        ofPoint    newCentre;
        
        // Check if the destination is above the source, in which case we determine the
        // anchors in the reverse order.
        if (aCentre.y < yPos)
        {
            anchorHere = calculateClosestAnchor(fromHere, true, false, toThere);
            anchorThere = calculateAnchorForPoint(newCentre, kAnchorBottomCentre == anchorHere,
                                                  toThere, aCentre);
        }
        else
        {
            anchorThere = calculateAnchorForPoint(newCentre, false, toThere, aCentre);
            anchorHere = calculateClosestAnchor(fromHere, true, kAnchorBottomCentre == anchorThere,
                                                toThere);
        }
        if (isUDP)
        {
            ofSetColor(ServiceViewerApp::getUdpConnectionColor());
        }
        else
        {
            ofSetColor(ServiceViewerApp::getTcpConnectionColor());
        }
        ofSetLineWidth(ServiceViewerApp::getNormalConnectionWidth());
        DrawBezier(fromHere, toThere, aCentre, newCentre);
        ofSetLineWidth(1);
        drawSourceAnchor(anchorHere, fromHere);
        drawTargetAnchor(anchorThere, toThere);
    }
    ODL_OBJEXIT(); //####
} // PortEntry::drawDragLine

void
PortEntry::drawSourceAnchor(const AnchorSide anchor,
                            const ofPoint &  anchorPos)
{
    ODL_ENTER(); //####
    ODL_L1("anchor = ", static_cast<int> (anchor)); //####
    ODL_P1("anchorPos = ", &anchorPos); //####
    ofPoint first;
    ofPoint second;
    
    switch (anchor)
    {
        case kAnchorLeft :
            first = anchorPos + ofPoint(kArrowSize, -kArrowSize);
            second = anchorPos + ofPoint(kArrowSize, kArrowSize);
            break;
            
        case kAnchorRight :
            first = anchorPos + ofPoint(-kArrowSize, -kArrowSize);
            second = anchorPos + ofPoint(-kArrowSize, kArrowSize);
            break;
            
        case kAnchorBottomCentre :
            first = anchorPos + ofPoint(-kArrowSize, -kArrowSize);
            second = anchorPos + ofPoint(kArrowSize, -kArrowSize);
            break;
            
        case kAnchorTopCentre :
            first = anchorPos + ofPoint(-kArrowSize, kArrowSize);
            second = anchorPos + ofPoint(kArrowSize, kArrowSize);
            break;
            
        default :
            break;
            
    }
    if (kAnchorUnknown != anchor)
    {
        ofLine(anchorPos, first);
        ofLine(anchorPos, second);
    }
    ODL_EXIT(); //####
} // PortEntry::drawSourceAnchor

void
PortEntry::drawTargetAnchor(const AnchorSide anchor,
                            const ofPoint &  anchorPos)
{
    ODL_ENTER(); //####
    ODL_L1("anchor = ", static_cast<int> (anchor)); //####
    ODL_P1("anchorPos = ", &anchorPos); //####
    ofPoint first;
    ofPoint second;
    
    switch (anchor)
    {
        case kAnchorLeft :
            first = anchorPos + ofPoint(-kArrowSize, -kArrowSize);
            second = anchorPos + ofPoint(-kArrowSize, kArrowSize);
            break;
            
        case kAnchorRight :
            first = anchorPos + ofPoint(kArrowSize, -kArrowSize);
            second = anchorPos + ofPoint(kArrowSize, kArrowSize);
            break;
            
        case kAnchorBottomCentre :
            first = anchorPos + ofPoint(-kArrowSize, kArrowSize);
            second = anchorPos + ofPoint(kArrowSize, kArrowSize);
            break;
            
        case kAnchorTopCentre :
            first = anchorPos + ofPoint(-kArrowSize, -kArrowSize);
            second = anchorPos + ofPoint(kArrowSize, -kArrowSize);
            break;
            
        default :
            break;
            
    }
    if (kAnchorUnknown != anchor)
    {
        ofLine(anchorPos, first);
        ofLine(anchorPos, second);
    }
    ODL_EXIT(); //####
} // PortEntry::drawTargetAnchor

ofPoint
PortEntry::getCentre(void)
{
    ODL_OBJENTER(); //####
    ofRectangle outer(getShape());
    
    ODL_OBJEXIT(); //####
    return ofPoint(outer.x + (outer.width / 2), outer.y + (outer.height / 2));
} // PortEntry::getCentre

bool
PortEntry::isPointInside(const ofPoint & aPoint)
const
{
    ODL_OBJENTER(); //####
    ODL_P1("aPoint = ", &aPoint); //####
    bool result = b.inside(aPoint);
    
    ODL_OBJEXIT_B(result); //####
    return result;
} // PortEntry::isPointInside

bool
PortEntry::isPointInside(const float xPos,
                         const float yPos)
const
{
    ODL_OBJENTER(); //####
    ODL_D2("xPos = ", xPos, "yPos = ", yPos); //####
    bool result = b.inside(xPos, yPos);
    
    ODL_OBJEXIT_B(result); //####
    return result;
} // PortEntry::isPointInside

bool
PortEntry::mouseDragged(ofMouseEventArgs & args)
{
    ODL_OBJENTER(); //####
    ODL_P1("args = ", &args); //####
    ODL_L1("args.button = ", args.button); //####
    bool result = inherited::mouseDragged(args);
    
    if (! result)
    {
        ServiceViewerApp & owner = getParent()->getOwner();
        
        if (b.inside(args.x, args.y))
        {
            bool addIsActive = owner.addIsActive();
            bool controlWasActive = owner.controlActive();
            bool removeIsActive = owner.removeIsActive();
            
            ODL_B3("addIsActive = ", addIsActive, "controlWasActive = ", controlWasActive, //####
                      "removeIsActive = ", removeIsActive); //####
            if (OF_MOUSE_BUTTON_3 == args.button)
            {
                result = true;
            }
            else if (controlWasActive)
            {
                result = true;
            }
            else if (addIsActive || removeIsActive)
            {
                result = true;
            }
        }
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // PortEntry::mouseDragged

bool
PortEntry::mouseMoved(ofMouseEventArgs & args)
{
    ODL_OBJENTER(); //####
    ODL_P1("args = ", &args); //####
    bool result = inherited::mouseMoved(args);
    
    ODL_OBJEXIT_B(result); //####
    return result;
} // PortEntry::mouseMoved

bool
PortEntry::mousePressed(ofMouseEventArgs & args)
{
    ODL_OBJENTER(); //####
    ODL_P1("args = ", &args); //####
    ODL_L1("args.button = ", args.button); //####
    bool result = inherited::mousePressed(args);
    
    if (! result)
    {
        ServiceViewerApp & owner = getParent()->getOwner();
        
        if (b.inside(args.x, args.y))
        {
            bool addIsActive = owner.addIsActive();
            bool controlWasActive = owner.controlActive();
            bool removeIsActive = owner.removeIsActive();
            
            ODL_B3("addIsActive = ", addIsActive, "controlWasActive = ", controlWasActive, //####
                      "removeIsActive = ", removeIsActive); //####
            if (OF_MOUSE_BUTTON_3 == args.button)
            {
                string prefix;
                string suffix;
                
                switch (MplusM::Utilities::GetPortKind(_portName.c_str()))
                {
                    case MplusM::Utilities::kPortKindAdapter :
                        prefix = "Adapter port ";
                        break;
                        
                    case MplusM::Utilities::kPortKindClient :
                        prefix = "Client port ";
                        break;
                        
                    case MplusM::Utilities::kPortKindRegistryService :
                        prefix = "Registry Service port ";
                        break;
                        
                    case MplusM::Utilities::kPortKindService :
                        prefix = "Service port ";
                        break;
                        
                    case MplusM::Utilities::kPortKindStandard :
                        prefix = "Standard port ";
                        break;
                        
                    case MplusM::Utilities::kPortKindUnknown :
                        break;
                        
                }
                if (0 < _portProtocol.size())
                {
                    suffix = "\nProtocol = '" + _portProtocol + "'";
                }
                ofSystemAlertDialog(prefix + _portName + suffix);
                result = true;
            }
            else if (controlWasActive)
            {
                result = true;
            }
            else if (addIsActive || removeIsActive)
            {
                owner.reportPortEntryClicked(this);
                result = true;
            }
        }
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // PortEntry::mousePressed

bool
PortEntry::mouseReleased(ofMouseEventArgs & args)
{
    ODL_OBJENTER(); //####
    ODL_P1("args = ", &args); //####
    ODL_L1("args.button = ", args.button); //####
    bool result = inherited::mouseReleased(args);
    
    if (! result)
    {
        ServiceViewerApp & owner = getParent()->getOwner();
        
        if (b.inside(args.x, args.y))
        {
            bool addIsActive = owner.addIsActive();
            bool controlWasActive = owner.controlActive();
            bool removeIsActive = owner.removeIsActive();
            
            ODL_B3("addIsActive = ", addIsActive, "controlWasActive = ", controlWasActive, //####
                      "removeIsActive = ", removeIsActive); //####
            if (OF_MOUSE_BUTTON_3 == args.button)
            {
                result = true;
            }
            else if (controlWasActive)
            {
                result = true;
            }
            else if (addIsActive)
            {
                if (owner.dragActive())
                {
                    owner.reportPortEntryClicked(this);
                    owner.clearDragState();
                }
                result = true;
            }
            else if (removeIsActive)
            {
                result = true;
            }
        }
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // PortEntry::mouseReleased

void
PortEntry::removeInputConnection(PortEntry * other)
{
    ODL_OBJENTER(); //####
    ODL_P1("other = ", other); //####
    if (other)
    {
        Connections::iterator walker(_inputConnections.begin());
        
        for ( ; _inputConnections.end() != walker; ++walker)
        {
            if (walker->_otherPort == other)
            {
                break;
            }
            
        }
        if (_inputConnections.end() != walker)
        {
            _inputConnections.erase(walker);
        }
    }
    ODL_OBJEXIT(); //####
} // PortEntry::removeInputConnection

void
PortEntry::removeOutputConnection(PortEntry * other)
{
    ODL_OBJENTER(); //####
    ODL_P1("other = ", other); //####
    if (other)
    {
        Connections::iterator walker(_outputConnections.begin());
        
        for ( ; _outputConnections.end() != walker; ++walker)
        {
            if (walker->_otherPort == other)
            {
                break;
            }
        }
        if (_outputConnections.end() != walker)
        {
            _outputConnections.erase(walker);
        }
    }
    ODL_OBJEXIT(); //####
} // PortEntry::removeOutputConnection

PortEntry *
PortEntry::setup(string      label,
                 const float width,
                 const float height)
{
    ODL_OBJENTER(); //####
    ODL_S1s("label = ", label); //####
    ODL_D2("width = ", width, "height = ", height); //####
    string tag;
    
    _portName = label;
    switch (_direction)
    {
        case kPortDirectionInput :
            tag = "In";
            break;
            
        case kPortDirectionInputOutput :
            tag = "I/O";
            break;
            
        case kPortDirectionOutput :
            tag = "Out";
            break;
            
        default :
            tag = "Unk";
            break;
            
    }
    PortEntry * result = static_cast<PortEntry *> (inherited::setup(tag, label, width, height));
    
    ODL_OBJEXIT_P(result); //####
    return result;
} // PortEntry::setup

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
