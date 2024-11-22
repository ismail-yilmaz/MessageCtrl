#include <MessageCtrl/MessageCtrl.h>

using namespace Upp;

class Messages : public TopWindow {
	MessageCtrl msg;
	DocEdit editor;
	Button  button1,  button2;
	
public:
	Messages()
	{
		Title("U++ Message Boxes (Passive Notifications)");
		SetRect(0,0, 640, 480);
		Sizeable().Zoomable().CenterScreen();
		SetMinSize({100, 100});

		auto action = [=](int id) {
			switch(id) {
			case IDYES: PromptOK("You've chosen 'yes'"); break;
			case IDNO:  PromptOK("You've chosen 'no'"); break;
			}
		};
		
		Add(editor.HSizePosZ().VSizePos(0, 24));
		Add(button1.SetLabel("Test").RightPos(4).BottomPos(4));
		Add(button2.SetLabel("Clear").LeftPos(4).BottomPos(4));

		button2 << [=] { msg.Clear(this); }; // Selective clearing.
		button1 << [=] {
			msg.Animation()
			  .Top()
			  .Information(*this, "This is a time-constrained information message. It will disappear in 5 seconds.", Null, 5)
			  .OK(*this, "This is a success message.")
			  .Warning(*this, "This is a warning message.")
			  .Error(*this, "This is an error message.")
			  .Bottom().NoIcon()
			  .AskYesNo(editor, "This is a question box 'in' the text editor with "
			                   "[^https:www`.ultimatepp`.org^ l`i`n`k]"
			                   " support. Would you like to continue?",
			                   action,
			                   callback(LaunchWebBrowser)
			);
			
		};
	}
};

GUI_APP_MAIN
{
	Messages().Run();
}