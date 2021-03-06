//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ctextlabel.h"
#include "../platform/iplatformfont.h"

namespace VSTGUI {

IdStringPtr CTextLabel::kMsgTruncatedTextChanged = "CTextLabel::kMsgTruncatedTextChanged";

//------------------------------------------------------------------------
// CTextLabel
//------------------------------------------------------------------------
/*! @class CTextLabel
*/
//------------------------------------------------------------------------
/**
 * CTextLabel constructor.
 * @param size the size of this view
 * @param txt the initial text as c string (UTF-8 encoded)
 * @param background the background bitmap
 * @param style the display style (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CRect& size, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CParamDisplay (size, background, style)
, text (0)
, textTruncateMode (kTruncateNone)
{
	setText (txt);
}

//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CTextLabel& v)
: CParamDisplay (v)
, text (0)
, textTruncateMode (v.textTruncateMode)
{
	setText (v.getText ());
}

//------------------------------------------------------------------------
CTextLabel::~CTextLabel ()
{
}

//------------------------------------------------------------------------
void CTextLabel::setText (UTF8StringPtr txt)
{
	if (txt && UTF8StringView (txt) == text)
		return;
	text = txt;
	if (textTruncateMode != kTruncateNone)
		calculateTruncatedText ();
	setDirty (true);
}

//------------------------------------------------------------------------
void CTextLabel::setTextTruncateMode (TextTruncateMode mode)
{
	if (textTruncateMode != mode)
	{
		textTruncateMode = mode;
		calculateTruncatedText ();
	}
}

//------------------------------------------------------------------------
void CTextLabel::calculateTruncatedText ()
{
	truncatedText = "";
	if (textRotation != 0.) // currently truncation is only supported when not rotated
		return;
	std::string _truncatedText;
	if (!(textTruncateMode == kTruncateNone || text.getByteCount () == 0 || fontID == 0 || fontID->getPlatformFont () == 0 || fontID->getPlatformFont ()->getPainter () == 0))
	{
		IFontPainter* painter = fontID->getPlatformFont ()->getPainter ();
		CCoord width = painter->getStringWidth (0, text.getPlatformString (), true);
		width += textInset.x * 2;
		if (width > getWidth ())
		{
			if (textTruncateMode == kTruncateTail)
			{
				_truncatedText = text;
				_truncatedText += "..";
				while (width > getWidth () && _truncatedText.size () > 2)
				{
					UTF8CharacterIterator it (_truncatedText);
					it.end ();
					for (int32_t i = 0; i < 3; i++, --it)
					{
						if (it == it.front ())
						{
							break;
						}
					}
					_truncatedText.erase (_truncatedText.size () - (2 + it.getByteLength ()), it.getByteLength ());
					width = painter->getStringWidth (0, CString (_truncatedText.c_str ()).getPlatformString (), true);
					width += textInset.x * 2;
				}
			}
			else if (textTruncateMode == kTruncateHead)
			{
				_truncatedText = "..";
				_truncatedText += text;
				while (width > getWidth () && _truncatedText.size () > 2)
				{
					UTF8CharacterIterator it (_truncatedText);
					for (int32_t i = 0; i < 2; i++, ++it)
					{
						if (it == it.back ())
						{
							break;
						}
					}
					_truncatedText.erase (2, it.getByteLength ());
					width = painter->getStringWidth (0, CString (_truncatedText.c_str ()).getPlatformString (), true);
					width += textInset.x * 2;
				}
			}
		}
	}
	if (truncatedText != _truncatedText)
	{
		truncatedText = _truncatedText.c_str ();
		changed (kMsgTruncatedTextChanged);
	}
}

//------------------------------------------------------------------------
const UTF8String& CTextLabel::getText () const
{
	return text;
}

//------------------------------------------------------------------------
void CTextLabel::draw (CDrawContext *pContext)
{
	drawBack (pContext);
	drawPlatformText (pContext, truncatedText.empty () ? text.getPlatformString () : truncatedText.getPlatformString ());
	setDirty (false);
}

//------------------------------------------------------------------------
bool CTextLabel::sizeToFit ()
{
	if (fontID == 0 || fontID->getPlatformFont () == 0 || fontID->getPlatformFont ()->getPainter () == 0)
		return false;
	CCoord width = fontID->getPlatformFont ()->getPainter ()->getStringWidth (0, text.getPlatformString (), true);
	if (width > 0)
	{
		width += (getTextInset ().x * 2.);
		CRect newSize = getViewSize ();
		newSize.setWidth (width);
		setViewSize (newSize);
		setMouseableArea (newSize);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CTextLabel::setViewSize (const CRect& rect, bool invalid)
{
	CRect current (getViewSize ());
	CParamDisplay::setViewSize (rect, invalid);
	if (textTruncateMode != kTruncateNone && current.getWidth () != getWidth ())
	{
		calculateTruncatedText ();
	}
}

//------------------------------------------------------------------------
void CTextLabel::drawStyleChanged ()
{
	if (textTruncateMode != kTruncateNone)
	{
		calculateTruncatedText ();
	}
	CParamDisplay::drawStyleChanged ();
}

} // namespace
