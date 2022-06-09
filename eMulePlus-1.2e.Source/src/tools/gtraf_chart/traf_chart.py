"""
Usage:

    traf_chart.py [options] filehash db_directory

Options:
    -c             Show details for completed chunks.
    -u             Show users for incomplete chunks.
"""

import sys, getopt, os
from bsddb3 import db
import struct
from binascii import b2a_hex,a2b_hex

from reportlab.platypus import SimpleDocTemplate, Paragraph,Spacer, TableStyle, Table
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_LEFT, TA_CENTER, TA_RIGHT, TA_JUSTIFY
from reportlab.lib.units import inch,cm
from reportlab.platypus import figures
from reportlab.graphics.shapes import Drawing
from reportlab.graphics.charts.textlabels import Label
from reportlab.graphics.charts.barcharts import VerticalBarChart,HorizontalBarChart
from reportlab.lib import colors



styleHeading1 = ParagraphStyle(name='Heading1',
                              alignment = TA_CENTER,
                              fontName = 'Helvetica-Bold',
                              fontSize=20,
                              leading=22)
styleHeading2 = ParagraphStyle(name='Heading2',
                              alignment = TA_CENTER,
                              fontName = 'Helvetica-Bold',
                              fontSize=14,
                              leading=16)
styleSectionHeading = ParagraphStyle(name='SectionHeading',
                              alignment = TA_LEFT,
                              fontName = 'Helvetica-Oblique',
                              fontSize=14,
                              leading=16)
styleSectionNormal = ParagraphStyle(name='SectionNormal',
                              alignment = TA_LEFT,
                              fontName = 'Times-Roman',
                              fontSize=10,
                              leading=12,
                              leftIndent=40,
                              firstLineIndent=-15)

TS1 = TableStyle([
    ('FONT', (0,0), (-1,-1), 'Helvetica',8,10),
    ('LINEABOVE', (0,0), (-1,0), 1, colors.black),
    ('LINEBELOW', (0,0), (-1,0), 1.0, colors.black), # the one below header
##        ('LINEABOVE', (0,-1), (-1,-1), 1.0, colors.black), # the one above total
    ('FONT', (0,0), (-1,0), 'Helvetica-Bold',8,10),
    ('LINEBELOW', (0,-1), (-1,-1), 1, colors.black),
    ('LINEAFTER', (-1,0), (-1,-1), 0.5, colors.black),
    ('LINEBEFORE', (0,0), (0,-1), 0.5, colors.black),
    ('LINEBEFORE', (0,0), (-1,-1), 0.5, colors.black), # internal vertical
    ('ALIGN', (0,0), (-1,-1), 'CENTER'),
    ('TOPPADDING', (0,1), (-1,-1), 0),
    ('BOTTOMPADDING', (0,1), (-1,-1), 0),
    ])


class FileReporter:
    def __init__(self, filehash_s, homeDir, detail_incomplete, detail_completed,
                    protect_privacy=0):
        self.filehash_s = filehash_s
        self.filehash = a2b_hex(filehash_s)
        self.homeDir = homeDir
        self.users = None
        self.detail_incomplete = detail_incomplete
        self.detail_completed = detail_completed
        self.protect_privacy = protect_privacy

    def report(self):
        self.open_dbs()
        output_name = "%s.pdf" % self.filehash_s
        self.doc = SimpleDocTemplate(output_name)
        self.story = []
        self.get_file_chunks()
        self.load_users_for_file()
        self.load_traffic()

        self.print_header()
        self.print_chunks_chart()
        self.print_file_chunks()

        self.doc.build(self.story)

        self.close_dbs()

        os.system('start %s' % output_name)

    def print_header(self):
        title1 = 'File: %s' % self.hash_to_name(self.filehash)
        title2 = 'hash: %s' % b2a_hex(self.filehash).upper()
        p = Paragraph(title1, styleHeading1)
        self.story.append(p)
        p = Paragraph(title2, styleHeading2)
        self.story.append(p)
        self.story.append(Spacer(0.01,0.5*cm))

    def open_dbs(self):
        self.env = db.DBEnv()
        self.env.open(self.homeDir, db.DB_JOINENV)
        self.dbFileChunks = db.DB(self.env)
        self.dbFileChunks.open('Jumpstart.db', "File-Chunks", db.DB_UNKNOWN, db.DB_RDONLY)
        self.dbHashName = db.DB(self.env)
        self.dbHashName.open('Jumpstart.db', "Hash-Name", db.DB_UNKNOWN, db.DB_RDONLY)
        self.dbFileUserChunk = db.DB(self.env)
        self.dbFileUserChunk.open('Jumpstart.db', "FileUser-Chunk", db.DB_UNKNOWN, db.DB_RDONLY)
        self.dbFileUserChunk_Blocks = db.DB(self.env)
        self.dbFileUserChunk_Blocks.open('Jumpstart.db', "FileUserChunk-Blocks", db.DB_UNKNOWN, db.DB_RDONLY)


    def close_dbs(self):
        self.dbFileUserChunk_Blocks.close()
        self.dbFileUserChunk.close()
        self.dbHashName.close()
        self.dbFileChunks.close()
        self.env.close()

    def hash_to_name(self, hash):
        if self.protect_privacy:
            return shortened_hash(hash)
        else:
            return self.dbHashName.get(hash)[:-1]

    def print_chunks_chart(self):
        nChunks = len(self.filechunks)

        d = Drawing(400, 140)
        traffic_MB = [n/(1024*1024) for n in self.traffic]
        data = [traffic_MB]
        bc = VerticalBarChart()
##        bc.x = 0
##        bc.y = 10
        bc.height = 100
        bc.width = 370
        bc.data = data
        bc.categoryAxis.tickUp = 0.5
        bc.categoryAxis.tickDown = 1.5

        bc.categoryAxis.categoryNames = []
        for n in range(nChunks):
            if n%5 == 0:
                bc.categoryAxis.categoryNames.append( str(n) )
            else:
                bc.categoryAxis.categoryNames.append('')

        bc.bars.strokeWidth = 0.3
        bc.valueAxis.labels.fontSize = 6
        bc.categoryAxis.labels.fontSize = 6
        bc.bars[0].fillColor = colors.lightblue
        
        bc.valueAxis.valueMin = 0

        d.add(bc)
        drawing = GraphicsDrawing(d, 'Traffic per chunk in MB')

        self.story.append(drawing)
        self.story.append(Spacer(0.01,1.0*cm))

    def print_file_chunks(self):
        chunks = self.dbFileChunks.get(self.filehash)

        p = Paragraph('Summary', styleSectionHeading)
        self.story.append(p)
        self.story.append(Spacer(0.01,0.3*cm))

        s ='Number of chunks: %d' % len(chunks)
        p = Paragraph(s, styleSectionNormal)
        self.story.append(p)

        completed = []
        never_seen = []
        others = []
        for chunk_no in range(len(chunks)):
            chunk_val = struct.unpack('B', chunks[chunk_no])[0]
            if chunk_val==255:
                completed.append(chunk_no)
            elif chunk_val==0:
                never_seen.append(chunk_no)
            else:
                others.append(chunk_no)

        if len(completed) > 0:
            s = "Completed chunks: %s" % str(completed)
            p = Paragraph(s, styleSectionNormal)
            self.story.append(p)
        if len(never_seen) > 0:
            s = "Never seen chunks: %s" % never_seen
            p = Paragraph(s, styleSectionNormal)
            self.story.append(p)
        if len(others) > 0:
            s = "Incomplete chunks: %s" % others
            p = Paragraph(s, styleSectionNormal)
            self.story.append(p)
            self.story.append(Spacer(0.01,0.3*cm))

            data = [['Chunk', 'Times chunk was seen']]
            for ch in others:
                data.append([ ch, struct.unpack('B', chunks[ch])[0] ])
            colwidths = [50, 200]
            t = Table(data,colwidths)
            #t.hAlign = 'LEFT'
            t.setStyle(TS1)
            self.story.append(t)
            self.story.append(Spacer(0.01,0.3*cm))

            if self.detail_incomplete:
                data = [['Chunk', 'Users for chunk']]
                for ch in others:
                    ch_str = str(ch)
                    for nick in self.users_for_chunk(ch):
                        data.append([ ch_str, nick ])
                        ch_str = ''
                t = Table(data,colwidths)
                #t.hAlign = 'LEFT'
                t.setStyle(TS1)
                self.story.append(t)
                self.story.append(Spacer(0.01,0.3*cm))

        if self.detail_completed and (len(completed) > 0):
            self.story.append(Spacer(0.01,0.5*cm))
            p = Paragraph('Completed chunks details', styleSectionHeading)
            self.story.append(p)
            self.story.append(Spacer(0.01,0.3*cm))
            for ch in completed:
                self.detail_chunk(ch)

    def load_users_for_file(self):
        if self.users is None:
            self.users = []
            curs = self.dbFileUserChunk.cursor()
            rec = curs.first()
            while rec is not None:
                fileuser = rec[0]
                userhash = fileuser[16:]
                filehash = fileuser[:16]
                if self.filehash == filehash:
                    self.users.append(userhash)
                rec = curs.next()

    def users_for_chunk(self, chunk_no):
        users = []
        for userhash in self.users:
            key = self.filehash + userhash
            user_chunks = self.dbFileUserChunk.get(key)
            ch_val = struct.unpack('B', user_chunks[chunk_no])[0]
            #print "... %d ..." % ch_val
            if ch_val>0:
                users.append(userhash)
        return [self.hash_to_name(u) for u in users]

    def load_traffic(self, ):
        nChunks = len(self.filechunks)
        self.traffic = [0] * nChunks
        for userhash in self.users:
            key = self.filehash + userhash
            user_chunks = self.dbFileUserChunk.get(key)
            if user_chunks is None:
                print "warning - user_chunks is None"
                continue
            for chunk_no in range(nChunks):
                key = self.filehash + userhash + struct.pack('i', chunk_no)
                curs = self.dbFileUserChunk_Blocks.cursor()
                rec = curs.get(key, flags=db.DB_SET)
                start_sizes = []
                while rec is not None:
                    start_size = rec[1]
                    b_start = struct.unpack('i', start_size[:4])[0]
                    b_size= struct.unpack('i', start_size[4:])[0]
                    self.traffic[chunk_no] += b_size
                    rec = curs.get(key, flags=db.DB_NEXT_DUP)

    def get_file_chunks(self):
        self.filechunks = self.dbFileChunks.get(self.filehash)


    def detail_chunk(self, chunk_no):
        chunk_start = 9728000*chunk_no
        s = "Details for chunk %d (starting at %d)" % (chunk_no, chunk_start)
        p = Paragraph(s, styleSectionNormal)
        self.story.append(p)
        self.story.append(Spacer(0.01,0.1*cm))

        data = [['User','Offset','Offset from\nstart of file','Size']]
        for userhash in self.users:
            key = self.filehash + userhash + struct.pack('i', chunk_no)
            curs = self.dbFileUserChunk_Blocks.cursor()
            rec = curs.get(key, flags=db.DB_SET)
            start_sizes = []
            while rec is not None:
                start_size = rec[1]
                b_start = struct.unpack('i', start_size[:4])[0]
                b_size= struct.unpack('i', start_size[4:])[0]
                start_sizes.append( (b_start, b_size) )
                rec = curs.get(key, flags=db.DB_NEXT_DUP)

            if len(start_sizes) > 0:
                user_str = self.hash_to_name(userhash)
                for b_start, b_size in start_sizes:
                    data.append( [user_str, b_start-chunk_start, \
                                b_start, b_size ] )
                    user_str = ''

        colwidths = [150, 80, 80, 80]
        t = Table(data,colwidths)
        #t.hAlign = 'LEFT'
        t.setStyle(TS1)
        self.story.append(t)
        self.story.append(Spacer(0.01,0.7*cm))


def main():
    detail_incomplete = 0
    detail_completed = 0

    args = sys.argv[1:]
    if not args or ("-h" in args):
        print __doc__
        return
    try:
        opts, fileArgs = getopt.getopt(args, "cu")
    except getopt.GetoptError:
        print __doc__
        return
    if len(fileArgs) < 2:
        print __doc__
        return
    for opt, val in opts:
        if opt == "-u":
            detail_incomplete = 1
        elif opt == "-c":
            detail_completed = 1
    filehash_s, homeDir = fileArgs

    file_rep = FileReporter(filehash_s, homeDir, detail_incomplete, detail_completed)
    file_rep.report()

def shortened_hash(hash):
    s = b2a_hex(hash)
    return s[:4] + '...' + s[-4:]


class GraphicsDrawing(figures.Figure):
    """Lets you include reportlab/graphics drawings seamlessly,
    with the right numbering."""
    def __init__(self, drawing, caption=''):
        figures.Figure.__init__(self,
                                  drawing.width,
                                  drawing.height,
                                  caption
                                  )
        self.drawing = drawing

        self.drawHAlign = 'CENTER'

    def drawFigure(self):
        d = self.drawing
        d.wrap(d.width, d.height)
        d.drawOn(self.canv, 0, 0)

    def drawBorder(self):
        pass

##    def draw(self):
##        dx = self.dx
##        if self.drawHAlign == 'LEFT':
##            self.dx = 0
##        elif self.drawHAlign == 'RIGHT':
##            self.dx = 2*dx
##        figures.Figure.draw(self)
##        self.dx = dx



if __name__=='__main__':
    main()
