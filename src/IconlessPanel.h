//--------------------------------------------------------------------------------------
//
//  File:       IconlessPanel.h
//
//  Project:    M+M
//
//  Contains:   The class declaration for a GUI panel without save / restore icons.
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
//  Created:    2014-05-09
//
//--------------------------------------------------------------------------------------

#if (! defined(__ServiceViewer__IconlessPanel__))
# define __ServiceViewer__IconlessPanel__

# include "ofxGuiGroup.h"

# define SHOW_PANEL_HEADER_ /* */

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# endif // defined(__APPLE__)
/*! @file
 
 @brief The class declaration for a GUI panel without save / restore icons. */
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

/*! @brief A GUI panel without save / restore icons. */
class IconlessPanel : protected ofxGuiGroup
{
public:
    
    /*! @brief The constructor. */
	IconlessPanel(void);
    
    /*! @brief The constructor.
     @param parameters The set of named parameters for the panel.
     @param filename The settings file for the panel.
     @param xx The initial horizontal position of the panel.
     @param yy The initial vertical position of the panel. */
	IconlessPanel(const ofParameterGroup & parameters,
                  string                   filename = "settings.xml",
                  const float              xx = 10,
                  const float              yy = 10);
    
    /*! @brief The destructor. */
	virtual ~IconlessPanel(void);
    
    /*! @brief Set the parameters of the panel.
     @param collectionName The name of the panel.
     @param filename The settings file for the panel.
     @param xx The initial horizontal position of the panel.
     @param yy The initial vertical position of the panel. */
	IconlessPanel * setup(string      collectionName = "",
                          string      filename = "settings.xml",
                          const float xx = 10,
                          const float yy = 10);
    
    /*! @brief Set the parameters of the panel.
     @param parameters The set of named parameters for the panel.
     @param filename The settings file for the panel.
     @param xx The initial horizontal position of the panel.
     @param yy The initial vertical position of the panel. */
	IconlessPanel * setup(const ofParameterGroup & parameters,
                          string                   filename = "settings.xml",
                          const float              xx = 10,
                          const float              yy = 10);
    
    /*! @brief Return the width of the panel.
     @returns The width of the panel. */
	float getWidth(void)
    {
        return inherited::getWidth();
    } // getWidth
    
    /*! @brief Change the width of the panel.
     @param newWidth The new width for the panel. */
	void setWidth(const float newWidth);

protected:

    /*! @brief Prepare the panel for display. */
	virtual void generateDraw(void);
    
    /*! @brief Display the panel. */
	virtual void render(void);
    
private:

    /*! @brief The class that this class is derived from. */
    typedef ofxGuiGroup inherited;

    /*! @brief Copy constructor.
     
     Note - not implemented and private, to prevent unexpected copying.
     @param other Another object to construct from. */
    IconlessPanel(const IconlessPanel & other);
    
    /*! @brief Assignment operator.
     
     Note - not implemented and private, to prevent unexpected copying.
     @param other Another object to construct from. */
    IconlessPanel & operator=(const IconlessPanel & other);
    
    /*! @brief Return the width of the text to be displayed.
     @returns The width of the text to be displayed. */
    float calculateTextWidth(void);
        
}; // IconlessPanel

#endif // ! defined(__ServiceViewer__IconlessPanel__)