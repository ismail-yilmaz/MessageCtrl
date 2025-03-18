#include "MessageCtrl.h"

#define IMAGECLASS MessageCtrlImg
#define IMAGEFILE <MessageCtrl/MessageCtrl.iml>
#include <Draw/iml_source.h>

namespace Upp {

static bool operator||(MessageBox::Type a, MessageBox::Type b)
{
	return (int)a || int(b);
}

Button::Style CrossStyle()
{
	Button::Style s = Button::StyleNormal();
	for(int i = 0; i < 4; i++)
		s.look[i] = MessageCtrlImg::Get(i);
	s.pressoffset = Point(1, -1);
	return s;
}

void MessageBox::Set(Ctrl& c, const String& msg, bool animate, bool append, int secs)
{
	// Note: Color scheme is taken and modified from KMessageWidget.
	// See: https://api.kde.org/frameworks/kwidgetsaddons/html/kmessagewidget_8cpp_source.html

	int duration = clamp(secs, 0, 24 * 3600) * 1000;

	switch(msgtype) {
	case Type::INFORMATION:
		paper = AdjustIfDark(Blend(Color(128, 128, 255), Color(255, 255, 255)));
		icon  = CtrlImg::information();
		break;
	case Type::QUESTION:
		paper = AdjustIfDark(Blend(LtGray(), Color(239, 240, 241)));
		icon  = CtrlImg::question();
		break;
	case Type::WARNING:
		paper = AdjustIfDark(Blend(Color(246, 180, 0), Color(239, 240, 241)));
		icon  = CtrlImg::exclamation();
		break;
	case Type::SUCCESS:
		paper = AdjustIfDark(Blend(Color(39, 170, 96), Color(239, 240, 241)));
		icon  = CtrlImg::information();
		break;
	case Type::FAILURE:
		paper = AdjustIfDark(Blend(Color(218, 68, 83), Color(239, 240, 241)));
		icon  = CtrlImg::error();
		break;
	default:
		break;
	}

	SetFrame(FieldFrame());

	discarded   = false;
	ctrl.parent = &c;

	if(append)  c.AddFrame(*this);
	else		c.InsertFrame(0, *this);

	qtf.NoSb();
	qtf.VCenter();
	qtf.SetQTF(String("[G1 ") + msg);
	qtf.WhenLink = Proxy(WhenLink);

	int rpos = Zx(4);

	if(cross)
		SetCross(rpos);
	else {
		SetButtonLayout(bt1, id1, rpos);
		SetButtonLayout(bt2, id2, rpos);
		SetButtonLayout(bt3, id3, rpos);
	}
	Add(qtf.HSizePosZ(Zx((IsNull(icon) || !useicon) ? 4 : 24), rpos).VSizePosZ());

	if((animated = animate)) {
		Animate(ctrl, Rect(0, 0, c.GetSize().cx, GetHeight()), GUIEFFECT_SLIDE);
		animated = false;
	}
	else
		ctrl.SetRect(0, 0, c.GetSize().cx, GetHeight());

	if((msgtype == Type::INFORMATION || msgtype == Type::CUSTOM) && duration)
		tcb.Set(duration, [=] { Discard(); });
}

void MessageBox::SetButtonLayout(Button& b, int id, int& rpos)
{
	if(IsNull(b.GetLabel()))
		return;

	int fcy  = Draw::GetStdFontCy();
	int gap  = Zx(fcy / 4);
	int cx   = Zx(24);

	cx = max(2 * fcy + GetTextSize(b.GetLabel(), Draw::GetStdFont()).cx, cx);
	Add(b.RightPosZ(rpos, cx).VCenterPosZ(20));
	b << [=] { auto fn = WhenAction; Discard(); fn(id); };
	rpos += cx + gap;
}

void MessageBox::SetCross(int& rpos)
{
	int gap  = Zx(10 / 4);
	int cx   = Zx(10);
	btstyle  = CrossStyle();
	bt1.SetStyle(btstyle);
	Add(bt1.RightPosZ(rpos, cx).VCenterPosZ(cx));
	bt1 << [=] { auto fn = WhenAction; Discard(); fn(IDOK); };
	rpos += cx + gap;
}

void MessageBox::Discard()
{
	tcb.Kill();
	if(GetParent())
		GetParent()->RemoveFrame(*this);
	ctrl.SetRect(Null);
	ctrl.parent = nullptr;
	discarded   = true;
}

void MessageBox::FrameLayout(Rect& r)
{
	switch(place) {
	case Place::TOP: LayoutFrameTop(r, this, animated ? ctrl.GetSize().cy : GetHeight()); break;
	case Place::BOTTOM: LayoutFrameBottom(r, this, animated ? ctrl.GetSize().cy : GetHeight()); break;
	}
}

void MessageBox::FramePaint(Draw& w, const Rect& r)
{
	w.DrawRect(r, paper);

	if(useicon && !IsNull(icon)) {
		Size size = GetRatioSize(icon.GetSize(), Ctrl::VertLayoutZoom(16), 0);
		Rect rect = GetRect().CenterRect(size);
		w.DrawImage(Zx(4), rect.top, rect.Width(), rect.Height(), icon);
	}
}

void MessageBox::Dummy::Layout()
{
	if(parent) {
		parent->RefreshLayout();
		parent->Sync();
	}
}

MessageCtrl& MessageCtrl::Information(Ctrl& c, const String& s, Event<const String&> link, int sec)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::INFORMATION);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.UseCross();
	msg.Set(c, s, animate, append, sec);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::Warning(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::WARNING);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.UseCross();
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::OK(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::SUCCESS);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.UseCross();
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDNO, t_("No"));
	msg.ButtonL(IDYES, t_("Yes"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonM(IDNO, t_("No"));
	msg.ButtonL(IDYES,t_("Yes"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDRETRY,t_("Retry"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT, t_("Abort"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDIGNORE, t_("Ignore"));
	msg.ButtonM(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT,t_("Abort"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::Error(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.UseCross();
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDOK, t_("OK"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDNO, t_("No"));
	msg.ButtonL(IDYES, t_("Yes"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonM(IDNO, t_("No"));
	msg.ButtonL(IDYES,t_("Yes"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDRETRY,t_("Retry"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT, t_("Abort"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.UseIcon(icon);
	msg.ButtonR(IDIGNORE, t_("Ignore"));
	msg.ButtonM(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT,t_("Abort"));
	msg.Set(c, s, animate, append);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageBox& MessageCtrl::Create()
{
	messages.RemoveIf([=](int i) { return messages[i].IsDiscarded(); });
	return messages.Add();
}

void MessageCtrl::Remove(const MessageBox* m)
{
	messages.RemoveIf([=](int i) { return &messages[i] == m; });
}

void MessageCtrl::Clear(const Ctrl* c)
{
	if(!c) messages.Clear();
	else messages.RemoveIf([=](int i) { return messages[i].GetParent() == c; });
}

MessageCtrl::MessageCtrl()
: animate(false)
, append(false)
, icon(true)
, place(MessageBox::Place::TOP)

{
}
}