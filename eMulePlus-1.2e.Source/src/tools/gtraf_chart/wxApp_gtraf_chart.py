#!/usr/bin/env python
#Boa:App:BoaApp

from wxPython.wx import *

import wxFrame_gtraf_chart

modules ={'wxFrame_gtraf_chart': [1,
                         'Main frame of Application',
                         'wxFrame_gtraf_chart.py']}

class BoaApp(wxApp):
    def OnInit(self):
        wxInitAllImageHandlers()
        self.main = wxFrame_gtraf_chart.create(None)
        # needed when running from Boa under Windows 9X
        self.SetTopWindow(self.main)
        self.main.Show();self.main.Hide();self.main.Show()
        return True

def main():
    application = BoaApp(0)
    application.MainLoop()

if __name__ == '__main__':
    main()
