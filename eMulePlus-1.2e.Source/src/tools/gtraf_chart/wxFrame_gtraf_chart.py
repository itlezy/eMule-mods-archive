#Boa:Frame:wxFrame_gtraf_chart

from wxPython.wx import *

import os
from bsddb3 import db
from binascii import b2a_hex,a2b_hex
from traf_chart import FileReporter

def create(parent):
    return wxFrame_gtraf_chart(parent)

[wxID_WXFRAME_GTRAF_CHART, wxID_WXFRAME_GTRAF_CHARTBTNOPEN, 
 wxID_WXFRAME_GTRAF_CHARTBTNREPORT, wxID_WXFRAME_GTRAF_CHARTCHKPROTECTPRIVACY, 
 wxID_WXFRAME_GTRAF_CHARTCHKSHOWDETAILSCOMPL, 
 wxID_WXFRAME_GTRAF_CHARTCHKSHOWDETAILSINCOMPL, 
 wxID_WXFRAME_GTRAF_CHARTLISTFILES, wxID_WXFRAME_GTRAF_CHARTPANEL1, 
 wxID_WXFRAME_GTRAF_CHARTPANELSELECTIONS, wxID_WXFRAME_GTRAF_CHARTSTATICTEXT1, 
 wxID_WXFRAME_GTRAF_CHARTTXTDATABASEDIR, 
] = map(lambda _init_ctrls: wxNewId(), range(11))

class wxFrame_gtraf_chart(wxFrame):
    def _init_coll_listFiles_Columns(self, parent):
        # generated method, don't edit

        parent.InsertColumn(col=0, format=wxLIST_FORMAT_LEFT,
              heading='File Name', width=-1)
        parent.InsertColumn(col=1, format=wxLIST_FORMAT_LEFT,
              heading='File Hash', width=-1)

    def _init_utils(self):
        # generated method, don't edit
        pass

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wxFrame.__init__(self, id=wxID_WXFRAME_GTRAF_CHART,
              name='wxFrame_gtraf_chart', parent=prnt, pos=wxPoint(297, 279),
              size=wxSize(700, 405), style=wxDEFAULT_FRAME_STYLE,
              title='gtraf_chart')
        self._init_utils()
        self.SetClientSize(wxSize(692, 371))

        self.panel1 = wxPanel(id=wxID_WXFRAME_GTRAF_CHARTPANEL1, name='panel1',
              parent=self, pos=wxPoint(0, 0), size=wxSize(692, 371),
              style=wxTAB_TRAVERSAL)

        self.txtDatabaseDir = wxTextCtrl(id=wxID_WXFRAME_GTRAF_CHARTTXTDATABASEDIR,
              name='txtDatabaseDir', parent=self.panel1, pos=wxPoint(64, 40),
              size=wxSize(448, 21), style=wxTE_READONLY, value='')

        self.btnOpen = wxButton(id=wxID_WXFRAME_GTRAF_CHARTBTNOPEN,
              label='Open', name='btnOpen', parent=self.panel1, pos=wxPoint(520,
              40), size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.btnOpen, wxID_WXFRAME_GTRAF_CHARTBTNOPEN,
              self.OnBtnOpen)

        self.staticText1 = wxStaticText(id=wxID_WXFRAME_GTRAF_CHARTSTATICTEXT1,
              label='Database directory:', name='staticText1',
              parent=self.panel1, pos=wxPoint(64, 24), size=wxSize(368, 13),
              style=wxST_NO_AUTORESIZE)

        self.panelSelections = wxPanel(id=wxID_WXFRAME_GTRAF_CHARTPANELSELECTIONS,
              name='panelSelections', parent=self.panel1, pos=wxPoint(24, 96),
              size=wxSize(648, 264), style=wxTAB_TRAVERSAL)

        self.chkShowDetailsCompl = wxCheckBox(id=wxID_WXFRAME_GTRAF_CHARTCHKSHOWDETAILSCOMPL,
              label='Show details for completed chunks',
              name='chkShowDetailsCompl', parent=self.panelSelections,
              pos=wxPoint(24, 176), size=wxSize(256, 13), style=0)
        self.chkShowDetailsCompl.SetValue(False)

        self.chkShowDetailsIncompl = wxCheckBox(id=wxID_WXFRAME_GTRAF_CHARTCHKSHOWDETAILSINCOMPL,
              label='Show users for incomplete chunks',
              name='chkShowDetailsIncompl', parent=self.panelSelections,
              pos=wxPoint(24, 200), size=wxSize(256, 13), style=0)
        self.chkShowDetailsIncompl.SetValue(False)

        self.listFiles = wxListCtrl(id=wxID_WXFRAME_GTRAF_CHARTLISTFILES,
              name='listFiles', parent=self.panelSelections, pos=wxPoint(24, 8),
              size=wxSize(592, 160),
              style=wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_ICON,
              validator=wxDefaultValidator)
        self._init_coll_listFiles_Columns(self.listFiles)

        self.btnReport = wxButton(id=wxID_WXFRAME_GTRAF_CHARTBTNREPORT,
              label='Report', name='btnReport', parent=self.panelSelections,
              pos=wxPoint(448, 176), size=wxSize(152, 23), style=0)
        EVT_BUTTON(self.btnReport, wxID_WXFRAME_GTRAF_CHARTBTNREPORT,
              self.OnBtnReport)

        self.chkProtectPrivacy = wxCheckBox(id=wxID_WXFRAME_GTRAF_CHARTCHKPROTECTPRIVACY,
              label='Protect privacy', name='chkProtectPrivacy',
              parent=self.panelSelections, pos=wxPoint(24, 240),
              size=wxSize(256, 13), style=0)
        self.chkProtectPrivacy.SetValue(False)

    def __init__(self, parent):
        self._init_ctrls(parent)
        
        self.panelSelections.Enable(0)

    def OnBtnOpen(self, event):
        direct = self.txtDatabaseDir.GetValue()
        if direct=='':
            direct = os.getcwd()
        dlg = wxDirDialog(self, defaultPath=direct, message='Select DB directory')
        try:
            if dlg.ShowModal() == wxID_OK:
                direct = dlg.GetPath()
                self.txtDatabaseDir.SetValue(direct)
                self.open_db(direct)
        finally:
            dlg.Destroy()

    def open_db(self, direct):
        if not os.path.exists(os.path.join(direct, 'jumpstart.db')):
            wxMessageBox('This does not look like a JumpStart database directory', 
                    style=wxOK|wxICON_ERROR)
            return

        self.env = db.DBEnv()
        self.env.open(direct, db.DB_JOINENV)
        self.dbFileChunks = db.DB(self.env)
        self.dbFileChunks.open('Jumpstart.db', "File-Chunks", db.DB_UNKNOWN, db.DB_RDONLY)
        self.dbHashName = db.DB(self.env)
        self.dbHashName.open('Jumpstart.db', "Hash-Name", db.DB_UNKNOWN, db.DB_RDONLY)


        curs = self.dbFileChunks.cursor()
        rec = curs.first()
        while rec is not None:
            filehash = rec[0]
            #print b2a_hex(filehash)
            self.listFiles.Append([self.hash_to_name(filehash), b2a_hex(filehash)])
            rec = curs.next()
            
        self.dbHashName.close()
        self.dbFileChunks.close()
        self.env.close()

        self.panelSelections.Enable(1)

    def hash_to_name(self, hash):
        return self.dbHashName.get(hash)[:-1]

    def OnBtnReport(self, event):
        selections = self.get_selections()
        if len(selections)==0 or len(selections)>1:
            wxMessageBox('Please select one file for report', 
                    style=wxOK|wxICON_ERROR)
            return
        
        item_no = selections[0]
        item = self.listFiles.GetItem(item_no, 1)
        hash_asc = item.GetText()

        direct = self.txtDatabaseDir.GetValue()
        detail_incomplete = self.chkShowDetailsIncompl.IsChecked()
        detail_completed = self.chkShowDetailsCompl.IsChecked()
        protect_privacy = self.chkProtectPrivacy.IsChecked()
        
        file_rep = FileReporter(hash_asc, direct, detail_incomplete, 
                detail_completed,protect_privacy)
                
        wxBeginBusyCursor()
        file_rep.report()
        wxEndBusyCursor()
        
    def get_selections(self):
        selections = []
        item = -1
        while 1:
            item = self.listFiles.GetNextItem(item,
                                     wxLIST_NEXT_ALL,
                                     wxLIST_STATE_SELECTED)
            if item == -1:
                break
            else:
                selections.append(item)
        return selections




