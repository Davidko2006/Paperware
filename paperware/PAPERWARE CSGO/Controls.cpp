
#include "Controls.h"
#include "Menu.h"
#include "RenderManager.h"

#pragma region Base Control
void CControl::SetPosition(int x, int y)
{
	m_x = x;
	m_y = y;
}

void CControl::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

void CControl::GetSize(int &w, int &h)
{
	w = m_iWidth;
	h = m_iHeight;
}

bool CControl::Flag(int f)
{
	if (m_Flags & f)
		return true;
	else
		return false;
}

POINT CControl::GetAbsolutePos()
{
	POINT p;
	RECT client = parent->GetClientArea();
	if (parent)
	{
		p.x = m_x + client.left;
		p.y = m_y + client.top + 29;
	}

	return p;
}

void CControl::SetFileId(std::string fid)
{
	FileIdentifier = fid;
}
#pragma endregion Implementations of the Base control functions

#pragma region CheckBox
CCheckBox::CCheckBox()
{
	Checked = false;

	m_Flags = UIFlags::UI_Clickable | UIFlags::UI_Drawable | UIFlags::UI_SaveFile;
	m_iWidth = 10;
	m_iHeight = 10;

	FileControlType = UIControlTypes::UIC_CheckBox;
}

void CCheckBox::SetState(bool s)
{
	Checked = s;
}

bool CCheckBox::GetState()
{
	return Checked;
}

void CCheckBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	Render::GradientV(a.x, a.y, 14, 14, Color(32, 32, 32, 255), Color(48, 48, 48, 255));
	Render::Outline(a.x, a.y, 14, 14, Color(2, 2, 2, 255));

	if (hover)
	{
		Render::Clear(a.x, a.y, 14, 14, Color(0, 255, 0, 40));
		Render::Outline(a.x, a.y, 14, 14, Color(255, 255, 255, 40));
	}

	if (Checked)

	{
		Render::Clear(a.x, a.y, 14, 14, Color(0, 255, 0, 200));
		Render::Outline(a.x, a.y, 14, 14, Color(2, 2, 2, 255));
	}
}




void CCheckBox::OnUpdate() { m_iWidth = 10; m_iHeight = 10; }

void CCheckBox::OnClick()
{
	Checked = !Checked;
}
#pragma endregion Implementations of the Check Box functions

#pragma region Label
CLabel::CLabel()
{
	m_Flags = UIFlags::UI_Drawable;
	Text = "Default";
	FileIdentifier = "Default";
}

void CLabel::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	Render::Text(a.x, a.y, Color(245, 245, 245, 255), Render::Fonts::MenuBold, Text.c_str());
}

void CLabel::SetText(std::string text)
{
	Text = text;
}

void CLabel::OnUpdate() {}
void CLabel::OnClick() {}
#pragma endregion Implementations of the Label functions

#pragma region GroupBox
CGroupBox::CGroupBox()
{
	Items = 1;
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_RenderFirst;
	Text = "Default";
	FileIdentifier = "Default";
}

void outlined_text_bordered(int x, int y, int w, int h, int thickness, Color color, Color outlined, const char* text)
{
	RECT text_size = Render::GetTextSize(Render::Fonts::MenuBold, text);

	Render::Line(x, y, x + 12, y, outlined);
	Render::Line(x + 15 + text_size.right + 5, y, x + w, y, outlined);
	Render::Line(x, y, x, y + h, outlined);
	Render::Line(x, y + h, x + w, y + h, outlined);
	Render::Line(x + w, y, x + w, y + h, outlined);
}

void box_outlined_text_bordered(int x, int y, int w, int h, int thickness, Color color, Color outlined, const char* text)
{
	RECT text_size = Render::GetTextSize(Render::Fonts::MenuBold, text);
	Render::Text(x + 15, y - (text_size.bottom / 2), color, Render::Fonts::MenuBold, text);

	Render::Line(x, y, x + 12, y, outlined);
	Render::Line(x + 15 + text_size.right + 5, y, x + w, y, outlined);
	Render::Line(x, y, x, y + h, outlined);
	Render::Line(x, y + h, x + w, y + h, outlined);
	Render::Line(x + w, y, x + w, y + h, outlined);

	outlined_text_bordered(x - 1, y - 1, w + 2, h + 2, 1, Color(75, 75, 75, 255), Color(15, 15, 15, 255), text);

}

void CGroupBox::Draw(bool hover)
{
	POINT abs_pos = GetAbsolutePos();
	box_outlined_text_bordered(abs_pos.x + 13, abs_pos.y + 10, m_iWidth, m_iHeight, 1, Color(255, 255, 255, 255), Color(75, 75, 75, 255), Text.c_str());
}

void CGroupBox::SetText(std::string text)
{
	Text = text;
}

void CGroupBox::PlaceCheckBox(std::string Label, CTab *Tab, CControl* control)
{
	int x = m_x + 29;
	int y = m_y + Items * 24;

	int cw, ch;
	control->SetPosition(x, y);
	control->GetSize(cw, ch);
	control->SetSize((m_iWidth / 2) - 32, ch);
	Tab->RegisterControl(control);

	x += 20;

	CLabel* label = new CLabel;
	label->SetPosition(x, y);
	label->SetText(Label);
	Tab->RegisterControl(label);
	Items++;
}

void CGroupBox::PlaceOtherControl(std::string Label, CTab *Tab, CControl* control)
{
	int x = m_x + 29;
	int y = m_y + Items * 24;

	int cw, ch;
	control->SetPosition(x, y);
	control->GetSize(cw, ch);
	control->SetSize((m_iWidth / 2) - 32, ch);
	Tab->RegisterControl(control);

	x += m_iWidth / 2 - 25;

	CLabel* label = new CLabel;
	label->SetPosition(x, y);
	label->SetText(Label);
	Tab->RegisterControl(label);
	Items++;
}

void CGroupBox::PlaceLabledControl(std::string Label, CTab *Tab, CControl* control)
{
	int x = m_x + 16;
	int y = m_y + Items * 24;

	CLabel* label = new CLabel;
	label->SetPosition(x, y);
	label->SetText(Label);
	Tab->RegisterControl(label);

	x += m_iWidth / 2;

	int cw, ch;
	control->SetPosition(x, y);
	control->GetSize(cw, ch);
	control->SetSize((m_iWidth / 2) - 32, ch);
	Tab->RegisterControl(control);
	Items++;
}

void CGroupBox::OnUpdate() {}
void CGroupBox::OnClick() {}
#pragma endregion Implementations of the Group Box functions

#pragma region Sliders
CSlider::CSlider()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;

	FileControlType = UIControlTypes::UIC_Slider;
}

void CSlider::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	float Ratio = Value / (Max - Min);
	float Location = Ratio*m_iWidth;


	if (hover)
	{

		Render::Outline(a.x, a.y, 14, 14, Color(255, 255, 255, 40));
	}

	Render::Outline(a.x + 1, a.y + 6, m_iWidth - 2, 9, Color(48, 48, 48, 255));
	Render::GradientV(a.x, a.y + 5, m_iWidth, 9, Color(51, 51, 51, 255), Color(31, 31, 31, 255));
	Render::GradientV(a.x, a.y + 5, Location, 9, Color(0, 255, 15, 200), Color(0, 255, 0, 200));
	Render::Outline(a.x, a.y + 5, m_iWidth, 9, Color(0, 0, 0, 255));


	char buffer[24];
	sprintf_s(buffer, "%.f", Value);
	RECT txtSize = Render::GetTextSize(Render::Fonts::MenuBold, buffer);
	Render::Text(a.x + Location - txtSize.right / 2, a.y + 10, Color(200, 200, 200, 255), Render::Fonts::MenuBold, buffer);
}

void CSlider::OnUpdate() {
	POINT a = GetAbsolutePos();
	m_iHeight = 11;

	if (DoDrag)
	{
		if (GUI.GetKeyState(VK_LBUTTON))
		{
			POINT m = GUI.GetMouse();
			float NewX;
			float Ratio;
			NewX = m.x - a.x - 1;
			if (NewX < 0) NewX = 0;
			if (NewX > m_iWidth) NewX = m_iWidth;
			Ratio = NewX / float(m_iWidth);
			Value = Min + (Max - Min)*Ratio;
		}
		else
		{
			DoDrag = false;
		}
	}
}

void CSlider::OnClick() {
	POINT a = GetAbsolutePos();
	RECT SliderRegion = { a.x, a.y, m_iWidth, 11 };
	if (GUI.IsMouseInRegion(SliderRegion))
	{
		DoDrag = true;
	}
}

float CSlider::GetValue()
{
	return Value;
}

void CSlider::SetValue(float v)
{
	Value = v;
}

void CSlider::SetBoundaries(float min, float max)
{
	Min = min; Max = max;
}

//2

CSlider1::CSlider1()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;

	FileControlType = UIControlTypes::UIC_Slider;
}

void CSlider1::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	float Ratio = Value / (Max - Min);
	float Location = Ratio*m_iWidth;

	//Draw::Outline(a.x + 1, a.y + 6, m_iWidth - 2, 9, Color(48, 48, 48, GUI.menu_alpha));
	Render::GradientV(a.x, a.y + 5, m_iWidth, 9, Color(100, 100, 100, 255), Color(100, 100, 100, 255));
	Render::Clear(a.x, a.y + 5, Location, 9, Color(116, 231, 100, 255));

	char buffer[24];
	sprintf_s(buffer, "%.2f", Value);
	RECT txtSize = Render::GetTextSize(Render::Fonts::MenuBold, buffer);
	Render::Text(a.x + Location - txtSize.right / 2, a.y + 10, Color(200, 200, 200, 255), Render::Fonts::MenuBold, buffer);
}

void CSlider1::OnUpdate() {
	POINT a = GetAbsolutePos();
	m_iHeight = 11;

	if (DoDrag)
	{
		if (GUI.GetKeyState(VK_LBUTTON))
		{
			POINT m = GUI.GetMouse();
			float NewX;
			float Ratio;
			NewX = m.x - a.x - 1;
			if (NewX < 0) NewX = 0;
			if (NewX > m_iWidth) NewX = m_iWidth;
			Ratio = NewX / float(m_iWidth);
			Value = Min + (Max - Min)*Ratio;
		}
		else
		{
			DoDrag = false;
		}
	}
}

void CSlider1::OnClick() {
	POINT a = GetAbsolutePos();
	RECT SliderRegion = { a.x, a.y, m_iWidth, 11 };
	if (GUI.IsMouseInRegion(SliderRegion))
	{
		DoDrag = true;
	}
}

float CSlider1::GetValue()
{
	return Value;
}

void CSlider1::SetValue(float v)
{
	Value = v;
}

void CSlider1::SetBoundaries1(float min, float max)
{
	Min = min; Max = max;
}
#pragma endregion Implementations of the Slider functions

#pragma region KeyBinders

char* KeyStrings[254] = { nullptr, "Left Mouse", "Right Mouse", "Control+Break", "Middle Mouse", "Mouse 4", "Mouse 5",
nullptr, "Backspace", "TAB", nullptr, nullptr, nullptr, "ENTER", nullptr, nullptr, "SHIFT", "CTRL", "ALT", "PAUSE",
"CAPS LOCK", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ESC", nullptr, nullptr, nullptr, nullptr, "SPACEBAR",
"PG UP", "PG DOWN", "END", "HOME", "Left", "Up", "Right", "Down", nullptr, "Print", nullptr, "Print Screen", "Insert",
"Delete", nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", "Left Windows", "Right Windows", nullptr, nullptr, nullptr, "NUM 0", "NUM 1", "NUM 2", "NUM 3", "NUM 4", "NUM 5", "NUM 6",
"NUM 7", "NUM 8", "NUM 9", "*", "+", "_", "-", ".", "/", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
"F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, "NUM LOCK", "SCROLL LOCK", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, "LSHIFT", "RSHIFT", "LCONTROL", "RCONTROL", "LMENU", "RMENU", nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Next Track", "Previous Track", "Stop", "Play/Pause", nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[{", "\\|", "}]", "'\"", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

CKeyBind::CKeyBind()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_KeyBind;
}

void CKeyBind::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	Render::GradientV(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(52, 52, 52, 255), Color(32, 32, 32, 255));
	Render::Outline(a.x, a.y, m_iWidth, 16, Color(0, 0, 0, 255));
	Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(48, 48, 48, 255));
	if (hover)
		Render::GradientV(a.x, a.y, m_iWidth, 16, Color(51, 51, 51, 255), Color(33, 33, 33, 255));
	Render::Outline(a.x, a.y, m_iWidth, 16, Color(0, 0, 0, 255));
	Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(48, 48, 48, 255));
	bool GoodKeyName = false;
	char NameBuffer[128];
	char* KeyName = "No Key";

	if (IsGettingKey)
	{
		KeyName = "Press Key";
	}
	else
	{
		if (Key >= 0)
		{
			KeyName = KeyStrings[Key];
			if (KeyName)
			{
				GoodKeyName = true;
			}
			else
			{
				if (GetKeyNameText(Key << 16, NameBuffer, 127))
				{
					KeyName = NameBuffer;
					GoodKeyName = true;
				}
			}
		}

		if (!GoodKeyName)
		{
			KeyName = "No Key";
		}
	}


	Render::Text(a.x + 4, a.y + 1, Color(255, 255, 255, 255), Render::Fonts::MenuBold, KeyName);
}

void CKeyBind::OnUpdate() {
	m_iHeight = 16;
	POINT a = GetAbsolutePos();
	if (IsGettingKey)
	{
		for (int i = 0; i < 255; i++)
		{
			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE)
				{
					IsGettingKey = false;
					Key = -1;
					return;
				}

				Key = i;
				IsGettingKey = false;
				return;
			}
		}
	}
}

void CKeyBind::OnClick() {
	POINT a = GetAbsolutePos();
	if (!IsGettingKey)
	{
		IsGettingKey = true;
	}
}

int CKeyBind::GetKey()
{
	return Key;
}

void CKeyBind::SetKey(int key)
{
	Key = key;
}

#pragma endregion Implementations of the KeyBind Control functions

#pragma region Button
CButton::CButton()
{
	m_iWidth = 177;
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable;
	Text = "Default";
	CallBack = nullptr;
	FileIdentifier = "Default";
}


void CButton::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	Render::Outline(a.x, a.y, m_iWidth, m_iHeight, Color(10, 10, 10, 255));
	if (hover)
		Render::GradientV(a.x + 1, a.y + 1, m_iWidth - 2, m_iHeight - 2, Color(52, 52, 52, 255), Color(32, 32, 32, 255));
	else
		Render::GradientV(a.x + 1, a.y + 1, m_iWidth - 2, m_iHeight - 2, Color(52, 52, 52, 255), Color(32, 32, 32, 255));

	RECT TextSize = Render::GetTextSize(Render::Fonts::MenuBold, Text.c_str());
	int TextX = a.x + (m_iWidth / 2) - (TextSize.left / 2);
	int TextY = a.y + (m_iHeight / 2) - (TextSize.bottom / 2);

	Render::Text(TextX, TextY, Color(255, 255, 255, 255), Render::Fonts::MenuBold, Text.c_str());
}

void CButton::SetText(std::string text)
{
	Text = text;
}

void CButton::SetCallback(CButton::ButtonCallback_t callback)
{
	CallBack = callback;
}

void CButton::OnUpdate()
{
	m_iHeight = 26;
}

void CButton::OnClick()
{
	if (CallBack)
		CallBack();
}
#pragma endregion Implementations of the Button functions

#pragma region ComboBox
CComboBox::CComboBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	//m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_ComboBox;
}

void CComboBox::Draw(bool hover)
{

	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 16 };
	Render::GradientV(a.x, a.y, m_iWidth, 16, Color(51, 51, 51, 255), Color(33, 33, 33, 255));
	Render::Outline(a.x, a.y, m_iWidth, 16, Color(0, 0, 0, 255));
	Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(48, 48, 48, 255));


	// Hover for the Top Box
	if (GUI.IsMouseInRegion(Region))
		Render::GradientV(a.x, a.y, m_iWidth, 16, Color(51, 51, 51, 255), Color(33, 33, 33, 255));
	Render::Outline(a.x, a.y, m_iWidth, 16, Color(0, 0, 0, 255));
	Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(48, 48, 48, 255));

	// If we have some items
	if (Items.size() > 0)
	{
		// The current item
		Render::Text(a.x + 4, a.y + 1, Color(255, 255, 255, 255), Render::Fonts::MenuBold, GetItem().c_str());

		// If the drop down part is open
		if (IsOpen)
		{
			Render::GradientV(a.x, a.y + 17, m_iWidth, Items.size() * 16, Color(51, 51, 51, 255), Color(33, 33, 33, 255));
			Render::Outline(a.x, a.y + 17, m_iWidth, Items.size() * 16, Color(9, 9, 9, 255));

			// Draw the items
			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 17 + i * 16, m_iWidth, 16 };


				if (GUI.IsMouseInRegion(ItemRegion))
				{
					Render::Text(a.x + 2, a.y + 19 + i * 16, Color(110, 244, 66, 255), Render::Fonts::MenuBold, Items[i].c_str());
				}
				else
				{
					Render::Text(a.x + 2, a.y + 19 + i * 16, Color(255, 255, 255, 255), Render::Fonts::MenuBold, Items[i].c_str());
				}
			}
		}
	}
	Vertex_t Verts2[3];
	Verts2[0].m_Position.x = a.x + m_iWidth - 10;
	Verts2[0].m_Position.y = a.y + 8;
	Verts2[1].m_Position.x = a.x + m_iWidth - 5;
	Verts2[1].m_Position.y = a.y + 8;
	Verts2[2].m_Position.x = a.x + m_iWidth - 7.5;
	Verts2[2].m_Position.y = a.y + 11;
	Render::Polygon(3, Verts2, Color(255, 255, 255, 255));
}

void CComboBox::AddItem(std::string text)
{
	Items.push_back(text);
	SelectedIndex = 0;
}

void CComboBox::OnUpdate()
{
	if (IsOpen)
	{
		m_iHeight = 16 + 16 * Items.size();

		if (parent->GetFocus() != this)
			IsOpen = false;
	}
	else
	{
		m_iHeight = 16;
	}

}

void CComboBox::OnClick()
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 16 };

	if (IsOpen)
	{
		// If we clicked one of the items(Not in the top bar)

		// Draw the items
		for (int i = 0; i < Items.size(); i++)
		{
			RECT ItemRegion = { a.x, a.y + 16 + i * 16, m_iWidth, 16 };

			// Hover
			if (GUI.IsMouseInRegion(ItemRegion))
			{
				SelectedIndex = i;
			}
		}


		/*if (!GUI.IsMouseInRegion(Region))
		{
		// Draw the items
		for (int i = 0; i < Items.size(); i++)
		{
		RECT ItemRegion = { a.x, a.y + 16 + i * 16, m_iWidth, 16 };

		// Hover
		if (GUI.IsMouseInRegion(ItemRegion))
		{
		SelectedIndex = i;
		}
		}
		}*/
		// Close the drop down
		IsOpen = false;
	}
	else
	{
		IsOpen = true;
	}
}

int CComboBox::GetIndex()
{
	return SelectedIndex;
}

std::string CComboBox::GetItem()
{
	if (SelectedIndex >= 0 && SelectedIndex < Items.size())
	{
		return Items[SelectedIndex];
	}

	return "Error";
}

void CComboBox::SelectIndex(int idx)
{
	if (idx >= 0 && idx < Items.size())
	{
		SelectedIndex = idx;
	}
}
CTextField::CTextField()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_KeyBind;
}

std::string CTextField::getText()
{
	return text;
}

void CTextField::SetText(std::string stext)
{
	text = stext;
}



void CTextField::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	Render::GradientV(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(52, 52, 52, 255), Color(32, 32, 32, 255));
	Render::Outline(a.x, a.y, m_iWidth, 16, Color(0, 0, 0, 255));
	Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(48, 48, 48, 255));
	if (hover || IsGettingKey)
	Render::GradientV(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(52, 52, 52, 255), Color(32, 32, 32, 255));

	const char *cstr = text.c_str();

	Render::Text(a.x + 2, a.y + 2, Color(255, 255, 255, 255), Render::Fonts::MenuBold, cstr);
}
char* KeyDigitss[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
void CTextField::OnUpdate()
{
	m_iHeight = 16;
	POINT a = GetAbsolutePos();
	POINT b;
	const char *strg = text.c_str();

	if (IsGettingKey)
	{
		b = GetAbsolutePos();
		for (int i = 0; i < 255; i++)
		{
			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
				{
					IsGettingKey = false;
					return;
				}

				if (i == VK_BACK && strlen(strg) != 0)
				{
					text = text.substr(0, strlen(strg) - 1);
				}

				if (strlen(strg) < 6 && (i == 0x30 || i == 0x31 || i == 0x32 || i == 0x33 || i == 0x34 || i == 0x35 || i == 0x36 || i == 0x37 || i == 0x38 || i == 0x39 || i == VK_NUMPAD0 || i == VK_NUMPAD1 || i == VK_NUMPAD2 || i == VK_NUMPAD3 || i == VK_NUMPAD4 || i == VK_NUMPAD5 || i == VK_NUMPAD6 || i == VK_NUMPAD7 || i == VK_NUMPAD8 || i == VK_NUMPAD9))
				{
					text = text + KeyDigitss[i];
					return;
				}
			}
		}
	}
}

void CTextField::OnClick()
{
	POINT a = GetAbsolutePos();
	if (!IsGettingKey)
	{
		IsGettingKey = true;
	}
}

#pragma endregion Implementation of the Textfield

#pragma region TextField2



CTextField2::CTextField2()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_KeyBind;
}

std::string CTextField2::getText()
{
	return text;
}

void CTextField2::SetText(std::string stext)
{
	text = stext;
}

void CTextField2::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	Render::GradientV(a.x, a.y, m_iWidth, m_iHeight, Color(30, 30, 30, 255), Color(40, 40, 40, 255));
	if (hover || IsGettingKey)
		Render::GradientV(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(30, 30, 30, 200), Color(40, 40, 40, 255));

	const char *cstr = text.c_str();

	Render::Text(a.x + 2, a.y + 2, Color(100, 100, 100, 255), Render::Fonts::MenuBold, cstr);
}

void CTextField2::OnUpdate()
{
	m_iHeight = 16;
	POINT a = GetAbsolutePos();
	POINT b;
	const char *strg = text.c_str();

	if (IsGettingKey)
	{
		b = GetAbsolutePos();
		for (int i = 0; i < 255; i++)
		{
			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
				{
					IsGettingKey = false;
					return;
				}

				if (i == VK_BACK && strlen(strg) != 0)
				{
					text = text.substr(0, strlen(strg) - 1);
				}

				if (strlen(strg) < 20 && i != NULL && KeyDigitss[i] != nullptr)
				{
					text = text + KeyDigitss[i];
					return;
				}

				if (strlen(strg) < 20 && i == 32)
				{
					text = text + " ";
					return;
				}
			}
		}
	}
}

void CTextField2::OnClick()
{
	POINT a = GetAbsolutePos();
	if (!IsGettingKey)
	{
		IsGettingKey = true;
	}
}

#pragma endregion Implementations of the ComboBox functions