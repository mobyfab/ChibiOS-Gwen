/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#ifndef GWEN_CONTROLS_WINDOWCONTROL_H
#define GWEN_CONTROLS_WINDOWCONTROL_H

#include "Gwen/Gwen.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/Dragger.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/ResizableControl.h"
#include "Gwen/Controls/Modal.h"
#include "Gwen/Controls/WindowButtons.h"
#include "Gwen/Skin.h"


namespace Gwen 
{
	namespace Controls
	{
		class CloseButton;

		class  WindowControl : public ResizableControl
		{
			public:

				GWEN_CONTROL( WindowControl, ResizableControl );

				virtual ~WindowControl();
				virtual void Render( Skin::Base* skin );
				virtual void RenderUnder( Skin::Base* skin );
#ifndef GWEN_NO_UNICODE
				virtual void SetTitle( Gwen::UnicodeString title );
				virtual void SetTitle( Gwen::String title ){ SetTitle( Gwen::Utility::StringToUnicode( title ) ); }
#else
				virtual void SetTitle( Gwen::String title );
#endif
				virtual void SetClosable(bool closeable);

				virtual void Touch();
				bool IsOnTop();

				virtual void SetHidden(bool hidden);

				void CloseButtonPressed();
				void RenderFocus( Gwen::Skin::Base* skin );
				void SetDeleteOnClose( bool b ){ m_bDeleteOnClose = b; }

				void MakeModal( bool bDrawBackground = true );
				void DestroyModal();
				
				static unsigned int GetCount(){return m_Count;};

				Gwen::Event::Caller	onWindowClosed;

			protected:

				Gwen::ControlsInternal::Dragger*	m_TitleBar;
				Gwen::Controls::Label*				m_Title;
				Gwen::Controls::WindowCloseButton*	m_CloseButton;

				bool m_bDeleteOnClose;

				ControlsInternal::Modal* m_Modal;
				static unsigned int m_Count;
		};
	}
}
#endif

