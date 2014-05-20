//--------------------------------------------------------------------------------------
//
//  File:       LabelWithShadow.h
//
//  Project:    M+M
//
//  Contains:   The class declaration for a text label with a shadow.
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
//  Created:    2014-05-12
//
//--------------------------------------------------------------------------------------

#if (! defined(__ServiceViewer__LabelWithShadow__))
# define __ServiceViewer__LabelWithShadow__

# include "ofxLabel.h"

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# endif // defined(__APPLE__)
/*! @file
 
 @brief The class declaration for a text label with a shadow. */
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

/*! @brief A text label with a shadow. */
class LabelWithShadow : public ofxLabel
{
public:
    
    /*! @brief The constructor. */
	LabelWithShadow(void);
    
    /*! @brief The constructor.
     @param label The named parameters of the panel.
     @param width The visual width of the panel.
     @param height The visual height of the panel. */
    LabelWithShadow(ofParameter<string> label,
                    const float         width = defaultWidth,
                    const float         height = defaultHeight);
    
    /*! @brief The destructor. */
	virtual ~LabelWithShadow(void);
    
    /*! @brief Return the color used for the text label shadow.
     @returns The color used for the text label shadow. */
	ofColor getShadowColor(void) const;
    
    /*! @brief Return the width of the text label shadow.
     @returns The width of the text label shadow. */
    float getShadowWidth(void) const;

    /*! @brief Set the color used for the text label shadow.
     @param color The color to be used for the text label shadow. */
	void setShadowColor(const ofColor & color);
    
    /*! @brief Set the width of the text label shadow.
     @param width The width of the text label shadow. */
    void setShadowWidth(const float width);
    
    /*! @brief Set the parameters of the panel.
     @param label The named parameters of the panel.
     @param width The visual width of the panel.
     @param height The visual height of the panel. */
    LabelWithShadow * setup(ofParameter<string> label,
                            const float         width = defaultWidth,
                            const float         height = defaultHeight);
    
    /*! @brief Set the parameters of the panel.
     @param labelName The name for the value of the panel.
     @param label The value of the panel.
     @param width The visual width of the panel.
     @param height The visual height of the panel. */
    LabelWithShadow * setup(string      labelName,
                            string      label,
                            const float width = defaultWidth,
                            const float height = defaultHeight);

    /*! @brief Set the default color for text label shadows.
     @param color The default color for text label shadows. */
	static void setDefaultShadowColor(const ofColor & color);

    /*! @brief Set the default width for text label shadows.
     @param width The default width for text label shadows. */
	static void setDefaultShadowWidth(const float width);

protected:
    
    /*! @brief Prepare the label for display. */
	virtual void generateDraw(void);

    /*! @brief Display the label. */
	virtual void render(void);
    
    /*! @brief The default text label shadow color. */
	static ofColor shadowColor;

    /*! @brief The default text label shadow width. */
    static float   shadowWidth;
    
    /*! @brief The text label shadow color for this label. */
	ofColor _thisShadowColor;

    /*! @brief The text label shadow width for this label. */
    float   _thisShadowWidth;

private:
    
    /*! @brief The class that this class is derived from. */
    typedef ofxLabel inherited;
    
    /*! @brief Copy constructor.
     
     Note - not implemented and private, to prevent unexpected copying.
     @param other Another object to construct from. */
    LabelWithShadow(const LabelWithShadow & other);
    
    /*! @brief Assignment operator.
     
     Note - not implemented and private, to prevent unexpected copying.
     @param other Another object to construct from. */
    LabelWithShadow & operator=(const LabelWithShadow & other);
    
    /*! @brief The visual representation of the shadow. */
	ofPath _shadow;
    
}; // LabelWithShadow

#endif // ! defined(__ServiceViewer__LabelWithShadow__)
