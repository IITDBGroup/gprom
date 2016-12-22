########################################
# Imports
from flask import Flask, render_template, request, redirect, url_for, abort, session
from gprom_wrapper import GProMWrapper
from hashlib import md5
from ansi2html import Ansi2HTMLConverter
import markdown
import os
from settings import APP_STATIC, APP_ROOT
from connsettings import *

########################################
# Setup flask
if GPROM_HOST == None or GPROM_USER == None or GPROM_PASSWD == None:
    raise Exception('Please set all connection options in connsettings.py')

app = Flask(__name__)
app.config['SECRET_KEY'] = 'F34TF$($e36D';
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///provgraph_web.db'
conv = Ansi2HTMLConverter()
w = GProMWrapper(user=GPROM_USER,passwd=GPROM_PASSWD,host=GPROM_HOST,port=GPROM_PORT,db=GPROM_DB,backend=GPROM_BACKEND)

########################################
# Load DB model
from models import *

########################################
# Index
@app.route('/')
def home():
    allQueries = db.session.query(ProvQuery).all()
    return render_template('index.html',allQueries=allQueries)

########################################
# Form processing a query
@app.route('/querysubmit', methods=['POST'])
def querysubmit():
    session['query'] = request.form['query']
    if request.form.has_key('gengraph'):
        session['action'] = 'graph'
    else:
        session['action'] = 'run'
    return redirect(url_for('showgraph'))

########################################
# Form loading a query
@app.route('/queryload', methods=['POST'])
def queryload():
    qid = request.form['storedQueryForm']
    oldquery = db.session.query(ProvQuery).filter(ProvQuery.id == qid).first()
    session['query'] = oldquery.query
    session['action'] = oldquery.graphOrResult
    return redirect(url_for('showgraph'))


########################################
# show query result or provenance graph
@app.route('/showgraph')
def showgraph():
    if not 'query' in session:
        return abort(403)
    query = session['query']
    action = session['action']
    # generate a graph
    if action == 'graph':
        queryResult=''
        queryhash = md5(query).hexdigest()
        imagefile = queryhash + '.svg'
        absImagepath = os.path.join(APP_STATIC, imagefile)
        if not(os.path.exists(absImagepath)):
            dotpath = os.path.join(APP_ROOT, 'pg.dot')        
            returncode, gpromlog, dotlog = w.generateProvGraph(query, absImagepath, dotpath)
            gpromlog = conv.convert(gpromlog,full=False)
            dotlog = conv.convert(dotlog,full=False)
        else:
            gpromlog, dotlog = '',''
            returncode = 0
    # output query results (translate into html table)
    else:
        returncode, gpromlog = w.runDLQuery(query)
        queryResult = gpromlog
        gpromlog = conv.convert(gpromlog,full=False)
        if returncode == 0:
            lines=queryResult.split('\n')
            numAttr=lines[0].count('|')
            lines=[ l for l in lines if not(not l or l.isspace()) ]
            lines=map(lambda x: '| ' + x + 'X', lines)
            if len(lines) > 1:
                lines[1] = '|' + (' -- | ' * numAttr)
            else:
                lines += ['|' + (' -- | ' * numAttr)]
            queryResult='\n'.join(lines)
            md = markdown.Markdown(extensions=['tables'])
            queryResult = md.convert(queryResult)
        dotlog, imagefile='',''
    # if query was successful then add to DB if we do not have it already
    if returncode == 0:
        existsAlready = db.session.query(ProvQuery).filter(ProvQuery.query == query and ProvQuery.graphOrResult == action).count()
        if existsAlready == 0:
            q = ProvQuery(query,imagefile,action)
            db.session.add(q)
            db.session.commit()
    allQueries = db.session.query(ProvQuery).all()
    return render_template('queryresult.html', query=query, gpromlog=gpromlog, dotlog=dotlog, imagefile=imagefile, returnedError=(returncode != 0), action=action, queryResult=queryResult,allQueries=allQueries)

if __name__ == '__main__':
    app.run()
