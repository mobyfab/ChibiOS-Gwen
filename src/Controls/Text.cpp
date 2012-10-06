/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Gwen.h"
#include "Gwen/Controls/Text.h"
#include "Gwen/Skin.h"
#include "Gwen/Utility.h"

using namespace Gwen;
using namespace Gwen::ControlsInternal;

GWEN_CONTROL_CONSTRUCTOR( Text )
{
	m_Font = NULL;
	m_ColorOverride = Color( 255, 255, 255, 0 );
	m_Color = GetSkin()->Colors.Label.Default;
	SetMouseInputEnabled( false );
	SetWrap( false );
}

Text::~Text()
{
	// NOTE: This font doesn't need to be released
	// Because it's a pointer to another font somewhere.
}

void Text::Layout( Skin::Base* skin )
{
	(void)skin;
	if ( m_bTextChanged )
	{
		RefreshSize();
		m_bTextChanged = false;
	}
}

Gwen::Font* Text::GetFont()
{
	return m_Font;
}

void Text::SetFont( Gwen::Font* pFont )
{
	if ( m_Font == pFont ) return;

	m_Font = pFont;
	m_bTextChanged = true;

	// Change the font of multilines too!
	{
		TextLines::iterator it = m_Lines.begin(); 
		TextLines::iterator itEnd = m_Lines.end();
		while ( it != itEnd )
		{
			(*it)->SetFont( m_Font );
			++it;
		}
	}

	Invalidate();
}

void Text::SetString( const TextObject& str )
{ 
	if ( m_String == str ) return;
#ifndef GWEN_NO_UNICODE
	m_String = str.GetUnicode(); 
#else
	m_String = str.Get(); 
#endif
	m_bTextChanged = true;
	Invalidate();
}

void Text::Render( Skin::Base* skin )
{
	if ( m_bWrap ) return;
	if ( Length() == 0 || !GetFont() ) return;

	if ( m_ColorOverride.a == 0 )
		skin->GetRender()->SetDrawColor( m_Color );
	else
		skin->GetRender()->SetDrawColor( m_ColorOverride );
#ifndef GWEN_NO_UNICODE
	skin->GetRender()->RenderText( GetFont(), Gwen::Point( GetPadding().left, GetPadding().top ), m_String.GetUnicode() );
#else
	skin->GetRender()->RenderText( GetFont(), Gwen::Point( GetPadding().left, GetPadding().top ), m_String.Get() );
#endif
}

Gwen::Rect Text::GetCharacterPosition( int iChar )
{
	if ( !m_Lines.empty() )
	{
		TextLines::iterator it = m_Lines.begin(); 
		TextLines::iterator itEnd = m_Lines.end();
		int iChars = 0;

		while ( it != itEnd )
		{
			Text* pLine = *it;
			++it;
			iChars += pLine->Length();

			if ( iChars <= iChar ) continue;

			iChars -= pLine->Length();

			Gwen::Rect rect = pLine->GetCharacterPosition( iChar - iChars );
			rect.x += pLine->X();
			rect.y += pLine->Y();

			return rect;
		}
	}

	if ( Length() == 0 || iChar == 0 )
	{
#ifndef GWEN_NO_UNICODE
		Gwen::Point p = GetSkin()->GetRender()->MeasureText( GetFont(), L" " );
#else
		Gwen::Point p = GetSkin()->GetRender()->MeasureText( GetFont(), " " );
#endif
		return Gwen::Rect( 1, 0, 0, p.y );
	}
#ifndef GWEN_NO_UNICODE
	UnicodeString sub = m_String.GetUnicode().substr( 0, iChar );
#else
	String sub = m_String.Get().substr( 0, iChar );
#endif
	Gwen::Point p = GetSkin()->GetRender()->MeasureText( GetFont(), sub );
	
	return Rect( p.x, 0, 0, p.y );
}

int Text::GetClosestCharacter( Gwen::Point p )
{
	if ( !m_Lines.empty() )
	{
		TextLines::iterator it = m_Lines.begin();
		TextLines::iterator itEnd = m_Lines.end();
		int iChars = 0;

		while ( it != itEnd )
		{
			Text* pLine = *it;
			++it;

			iChars += pLine->Length();

			if ( p.y < pLine->Y() ) continue;
			if ( p.y > pLine->Bottom() ) continue;

			iChars -= pLine->Length();

			int iLinePos = pLine->GetClosestCharacter( Gwen::Point( p.x - pLine->X(), p.y - pLine->Y() ) );
			//if ( iLinePos > 0 && iLinePos == pLine->Length() ) iLinePos--;
			iLinePos--;

			return iChars + iLinePos;
		}
	}


	int iDistance = 4096;
	int iChar = 0;
#ifndef GWEN_NO_UNICODE
	for ( size_t i=0; i<m_String.GetUnicode().length()+1; i++ )
#else
	for ( size_t i=0; i<m_String.Get().length()+1; i++ )
#endif
	{
		Gwen::Rect cp = GetCharacterPosition( i );
		int iDist = abs(cp.x - p.x) + abs(cp.y - p.y); // this isn't proper

		if ( iDist > iDistance ) continue;

		iDistance = iDist;
		iChar = i;
	}

	return iChar;
}

void Text::OnScaleChanged()
{
	Invalidate();
}

void Text::RefreshSize()
{
	if ( m_bWrap )
	{
		return RefreshSizeWrap();
	}

	if ( !GetFont() )
	{
		Debug::AssertCheck( 0, "Text::RefreshSize() - No Font!!\n" );
		return;
	}

	Gwen::Point p( 1, GetFont()->size );

	if ( Length() > 0 )
	{
#ifndef GWEN_NO_UNICODE
	p = GetSkin()->GetRender()->MeasureText( GetFont(), m_String.GetUnicode() );
#else
	p = GetSkin()->GetRender()->MeasureText( GetFont(), m_String.Get() );
#endif
	}

	p.x += GetPadding().left + GetPadding().right;
	p.y += GetPadding().top + GetPadding().bottom;

	if ( p.x == Width() && p.y == Height() ) 
		return;

	if ( p.y < GetFont()->size ) p.y = GetFont()->size;

	SetSize( p.x, p.y );
	InvalidateParent();
	Invalidate();
}
#ifndef GWEN_NO_UNICODE
void SplitWords(const Gwen::UnicodeString &s, wchar_t delim, std::vector<Gwen::UnicodeString> &elems) 
{
	Gwen::UnicodeString str;

	for ( int i=0; i<s.length(); i++ )
	{
		if ( s[i] == L'\n' )
		{
			if ( !str.empty() ) elems.push_back( str );
			elems.push_back( L"\n" );
			str.clear();
			continue;
		}

		if ( s[i] == L' ' )
		{
			str += s[i];
			elems.push_back( str );
			str.clear();
			continue;
		}

		str += s[i];
	}

	if ( !str.empty() ) elems.push_back( str );
}

void Text::RefreshSizeWrap()
{
	RemoveAllChildren();
	m_Lines.clear();

	std::vector<Gwen::UnicodeString> words;
	SplitWords( GetText().GetUnicode(), L' ', words );

	// Adding a bullshit word to the end simplifies the code below
	// which is anything but simple.
	words.push_back( L"" );

	if ( !GetFont() )
	{
		Debug::AssertCheck( 0, "Text::RefreshSize() - No Font!!\n" );
		return;
	}

	Point pFontSize = GetSkin()->GetRender()->MeasureText( GetFont(), L" " );

	int w = GetParent()->Width();
	int x = 0, y = 0;

	Gwen::UnicodeString strLine;

	std::vector<Gwen::UnicodeString>::iterator it = words.begin();
	for ( it; it != words.end(); ++it )
	{
		bool bFinishLine = false;
		bool bWrapped = false;

		// If this word is a newline - make a newline (we still add it to the text)
		if ( (*it).c_str()[0] == L'\n' ) bFinishLine = true;

		// Does adding this word drive us over the width?
		{
			strLine += (*it);
			Gwen::Point p = GetSkin()->GetRender()->MeasureText( GetFont(), strLine );
			if ( p.x > Width() ) { bFinishLine = true; bWrapped = true; }
		}

		// If this is the last word then finish the line
		if ( --words.end() == it )
		{
			bFinishLine = true;
		}

		if ( bFinishLine )
		{
			Text* t = new Text( this );
				t->SetFont( GetFont() );
				t->SetString( strLine.substr( 0, strLine.length() - (*it).length() ) );
				t->RefreshSize();
				t->SetPos( x, y );
			m_Lines.push_back( t );

			// newline should start with the word that was too big
			strLine = *it;

			// Position the newline
			y += pFontSize.y;
			x = 0;

			//if ( strLine[0] == L' ' ) x -= pFontSize.x;
		}

	}

	// Size to children height and parent width
	{
		Point childsize = ChildrenSize();
		SetSize( w, childsize.y );
	}

	InvalidateParent();
	Invalidate();
}
#else
void SplitWords(const Gwen::String &s, wchar_t delim, std::vector<Gwen::String> &elems) 
{
	Gwen::String str;
	(void)delim;

	for (unsigned int i=0; i<s.length(); i++ )
	{
		if ( s[i] == '\n' )
		{
			if ( !str.empty() ) elems.push_back( str );
			elems.push_back( "\n" );
			str.clear();
			continue;
		}

		if ( s[i] == ' ' )
		{
			str += s[i];
			elems.push_back( str );
			str.clear();
			continue;
		}

		str += s[i];
	}

	if ( !str.empty() ) elems.push_back( str );
}

void Text::RefreshSizeWrap()
{
	RemoveAllChildren();
	m_Lines.clear();

	std::vector<Gwen::String> words;
	SplitWords( GetText().Get(), ' ', words );

	// Adding a bullshit word to the end simplifies the code below
	// which is anything but simple.
	words.push_back( "" );

	if ( !GetFont() )
	{
		Debug::AssertCheck( 0, "Text::RefreshSize() - No Font!!\n" );
		return;
	}

	Point pFontSize = GetSkin()->GetRender()->MeasureText( GetFont(), " " );

	int w = GetParent()->Width();
	int x = 0, y = 0;

	Gwen::String strLine;

	std::vector<Gwen::String>::iterator it = words.begin();
	for ( ; it != words.end(); ++it )
	{
		bool bFinishLine = false;
		//bool bWrapped = false;

		// If this word is a newline - make a newline (we still add it to the text)
		if ( (*it).c_str()[0] == '\n' ) bFinishLine = true;

		// Does adding this word drive us over the width?
		{
			strLine += (*it);
			Gwen::Point p = GetSkin()->GetRender()->MeasureText( GetFont(), strLine );
			if ( p.x > Width() ) { bFinishLine = true; /*bWrapped = true;*/ }
		}

		// If this is the last word then finish the line
		if ( --words.end() == it )
		{
			bFinishLine = true;
		}

		if ( bFinishLine )
		{
			Text* t = new Text( this );
				t->SetFont( GetFont() );
				t->SetString( strLine.substr( 0, strLine.length() - (*it).length() ) );
				t->RefreshSize();
				t->SetPos( x, y );
				m_Lines.push_back( t );

			// newline should start with the word that was too big
			strLine = *it;

			// Position the newline
			y += pFontSize.y;
			x = 0;

			//if ( strLine[0] == ' ' ) x -= pFontSize.x;
		}

	}

	// Size to children height and parent width
	{
		Point childsize = ChildrenSize();
		SetSize( w, childsize.y );
	}

	InvalidateParent();
	Invalidate();
}
#endif

int Text::NumLines()
{
	return m_Lines.size();
}

Text* Text::GetLine( int i )
{
	TextLines::iterator it = m_Lines.begin();
	TextLines::iterator itEnd = m_Lines.end();

	while ( it != itEnd )
	{
		if ( i == 0 ) return *it;
		++it;
		i--;
	}

	return NULL;
}

int Text::GetLineFromChar( int i )
{
	TextLines::iterator it = m_Lines.begin();
	TextLines::iterator itEnd = m_Lines.end();
	int iChars = 0;
	int iLine = 0;

	while ( it != itEnd )
	{
		Text* pLine = *it;
		++it;

		iChars += pLine->Length();

		if ( iChars > i ) return iLine;
		iLine++;
	}

	return iLine;
}

int Text::GetStartCharFromLine( int i )
{
	TextLines::iterator it = m_Lines.begin();
	TextLines::iterator itEnd = m_Lines.end();
	int iChars = 0;

	while ( it != itEnd )
	{
		Text* pLine = *it;
		++it;
		if ( i == 0 ) return Gwen::Clamp( iChars, 0, Length() );

		iChars += pLine->Length();
		i--;
	}

	return Gwen::Clamp( iChars, 0, Length() );
}

int Text::GetEndCharFromLine( int i )
{
	int iStart = GetStartCharFromLine( i );
	Text* iLine = GetLine( i );

	if ( iLine )
	{
		iStart += iLine->Length();
	}

	return Gwen::Clamp( iStart, 0, Length() );
}

int Text::GetCharPosOnLine( int i )
{
	int iLine = GetLineFromChar( i );
	Text* line = GetLine( iLine );
	if ( !line ) return 0;

	int iStart = GetStartCharFromLine( iLine );

	return i - iStart;
}

