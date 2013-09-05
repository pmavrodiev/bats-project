import ply.lex as lex
import MySQLdb
#
db = MySQLdb.connect(host="localhost", # your host, usually localhost
                     user="root", # your username
                     passwd="1234567890", # your password
                     db="GB2_bats_new", # name of the data base
                     unix_socket="/tmp/mysql.sock"
                    )
class MyLexer:
    bats = []
    boxes = []
    year = 0    
    #token rules
    tokens = (
              "BATID",
              "EXPERIMENTALBOXID",
              "YEAR"
             )
    def t_YEAR(self,t):
        r"[2][0-9]{3}"
        self.year = t.value
        return t;
    def t_BATID(self,t):
        r"[a-zA-Z0-9]{10}"
        self.bats.append(t.value);
        return t;

    def t_EXPERIMENTALBOXID(self,t):
        r"[0-9]+"
        self.boxes.append(t.value);
        return t;

    def t_newline(self,t):
        r"\n+"
        t.lexer.lineno += len(t.value)
        t_ignore  = " \t"

    def t_error(self,t):
        print "Illegal character '%s'" % t.value[0]
        t.lexer.skip(1)    
    # Build the lexer
    def build(self,**kwargs):
        self.lexer = lex.lex(module=self, **kwargs)
    def read_chunks(self,file_object,chunk_size=1024):
        while True:
            data = file_object.read(chunk_size)
            if not data: break
            yield data
    def parse(self,filename):
        f = open(filename)
        for line in self.read_chunks(f):    
            self.lexer.input(line)
            while True:
                tok = self.lexer.token()
                if not tok: break              


p = MyLexer()
p.build()
p.parse("test.dat")
print p.boxes, p.year, p.bats

bats=[]
boxes=[]
cur = db.cursor()
cur.execute("SELECT DISTINCT id_bat FROM Findings where fin_date>\"2008-01-01\" and fin_date<\"2008-31-12\"")
for row in cur.fetchall():
    bats.append(row[0])
    
print bats