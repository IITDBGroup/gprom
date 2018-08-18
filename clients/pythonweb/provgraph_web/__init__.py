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
#import cx_Oracle

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
    # query="""SELECT * FROM all_tables WHERE owner = 'FGA_USER' AND tablespace_name = 'USERS' ORDER BY table_name;"""
    #query="""SELECT table_name FROM user_tables ORDER BY table_name;"""
    query="""SELECT * FROM tabcollist 
             WHERE table_name LIKE 'AC%' OR table_name LIKE 'AR%' 
                    OR table_name LIKE 'BANK%' OR table_name LIKE 'CRIMES%'
                    OR table_name LIKE 'EMPHIST%' OR table_name LIKE 'MOVIES%'
                    OR table_name LIKE 'ratings%' OR table_name LIKE 're%'
                    OR table_name LIKE 'SANEP%';"""
    returncode, gpromlog = w.runQuery(query)
    queryResult = gpromlog
    gpromlog = conv.convert(gpromlog,full=False)
    lines=[]
    if(returncode == 0):
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

    return render_template('index.html',allQueries=allQueries,returncode=returncode, queryResult=queryResult)
   # enumerate table lists in the database

#    query="""SELECT table_name FROM user_tables"""
#    query+=""" WHERE tablespace_name = 'USERS'"""
#    query+=""" AND num_rows < 30 AND avg_row_len < 30"""
#    query+=""" AND table_name NOT LIKE 'SYS%' AND min_extents IS NOT NULL AND ROWNUM <= 3"""
#    con = cx_Oracle.connect(GPROM_USER, GPROM_PASSWD, GPROM_HOST+':'+str(GPROM_PORT)+'/'+GPROM_DB)
#    cur = con.cursor()
#    cur.execute(query)
#    indblist=[]
#    for eachtable in cur.fetchall():
#	seperateEach=[]
#        tableName=''.join(eachtable)
#        query='SELECT * FROM ' + tableName  + ' WHERE ROWNUM < 5'
#	cur.execute(query)
# 	seperateEach=[tableName,'\n'.join([str(eachrow) for eachrow in cur.fetchall()])]
#	indblist+=['\n'.join([str(each) for each in seperateEach])]
#    con.close()
##    tables=[indblist.split(',')]
##    # reformat into table
##    if indblist != 0:
##        inlines=indblist.split('\n')
##	reline=lines[0:1]
##        reline=[ l for l in reline if not(not l or l.isspace()) ]
##	reline=map(lambda x: 'Database Relation ' + x, reline)
##	relDB=''.join(reline)
##        md = markdown.Markdown(reline[0], extensions=['markdown.extensions.smart_strong'])
##        relDB = md.convert(relDB)
##	insline=lines[1:]
##        numAttr=insline[0].count('|')
##        insline=[ l for l in insline if not(not l or l.isspace()) ]
##	insline=map(lambda x: '| ' + x + 'X', insline)
##        if len(insline) > 1:
##	    insline[1] = '|' + (' -- | ' * numAttr)
##        else:
##            insline += ['|' + (' -- | ' * numAttr)]
##        insDB='\n'.join(insline)
##        md = markdown.Markdown(extensions=['tables'])
##        insDB = md.convert(insDB)
#    return render_template('index.html', allQueries=allQueries, indblist=indblist)

########################################
# Form processing a query
@app.route('/querysubmit', methods=['POST'])
def querysubmit():
    session['query'] = request.form['query']

    if request.form['topk'] is not None:
        session['topk'] = request.form['topk']

    if request.form['sSize'] is not None:
        session['sSize'] = request.form['sSize']

    if request.form['fPattern'] is not None:
        session['fPattern'] = request.form['fPattern']

    if request.form['recall'] is not None:
        session['recall'] = request.form['recall']

    if request.form['info'] is not None:
        session['info'] = request.form['info']

    if request.form.has_key('genquery'):
        session['action'] = 'run'
    elif request.form.has_key('genprovgame'):
	    session['action'] = 'provgame'
    elif request.form.has_key('genprovgraph'):
        session['action'] = 'provgraph'
    elif request.form.has_key('genprovpolygraph'):
        session['action'] = 'provpolygraph'
    elif request.form.has_key('gentriograph'):
        session['action'] = 'triograph'
    elif request.form.has_key('genlingraph'):
        session['action'] = 'lingraph'
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
    
    topk,sSize = '',''
    if not 'topk' in session: pass
    else: topk = session['topk']

    if not 'sSize' in session: pass
    else: sSize = session['sSize']

    fPattern,recall,info = '','',''
    if not 'fPattern' in session: pass
    else: fPattern = session['fPattern']

    if not 'recall' in session: pass
    else: recall = session['recall']

    if not 'info' in session: pass
    else: info = session['info']

    # generate a graph
    provQuest = query.find('WHY')
    summRequest = query.find('SUMMARIZED')
    lines=[]
    queryResult = ''
    gpromlog, dotlog, imagefile='','',''

    if action == 'provgame' or action == 'provgraph' or action == 'provpolygraph' or action == 'triograph' or action == 'lingraph':
	   if provQuest > 0:
            summQuery = ''
            if summRequest < 0:
                if recall != '' and info == '':
                   summQuery += ' SCORE AS (' + recall + ' * recall)'
                elif recall == '' and info != '':
                   summQuery += ' SCORE AS (' + info + ' * informativeness)'
                elif recall != '' and info != '':
                   summQuery += ' SCORE AS (' + recall + ' * recall + ' + info + ' * informativeness)'
                #
                if topk != '':
                   summQuery += ' TOP ' + topk
                if fPattern != '':
                   summQuery += ' FOR FAILURE OF (' + fPattern + ')'
                if sSize != '':
                   summQuery += ' SUMMARIZED BY LCA WITH SAMPLE(' + sSize + ').'
            else:
                score = query.find('SCORE')
                if score > 0:
                    summQuery += ' ' + query[query.find('SCORE'):]
                else:
                    top = query.find('TOP')
                    if top > 0:
                        summQuery += ' ' + query[query.find('TOP'):]
            # if action == 'provgraph' and topk != '' and sSize != '':
            #   query = query[:query.find('))')] + ')) FORMAT REDUCED_GP. TOP ' + topk + ' SUMMARIZED BY LCA WITH SAMPLE(' + sSize + ').'
            if action == 'provgraph': #and topk == '' and sSize == '':
              query = query[:query.find('))')] + ')) FORMAT REDUCED_GP.'
#       graphFormat = query.find('FORMAT')
#       if graphFormat < 1:
#           query = query[:-1] + ' FORMAT REDUCED_GP.'
            # if action == 'provpolygraph' and topk != '' and sSize != '':
            #   query = query[:query.find('))')] + ')) FORMAT TUPLE_RULE_GOAL_TUPLE. TOP ' + topk + ' SUMMARIZED BY LCA WITH SAMPLE(' + sSize + ').'
            if action == 'provpolygraph': #and topk == '' and sSize == '':
              query = query[:query.find('))')] + ')) FORMAT TUPLE_RULE_GOAL_TUPLE.'
        # # 
            # if action == 'triograph' and topk != '' and sSize != '':
            #   query = query[:query.find('))')] + ')) FORMAT HEAD_RULE_EDB. TOP ' + topk + ' SUMMARIZED BY LCA WITH SAMPLE(' + sSize + ').'
            if action == 'triograph': #and topk == '' and sSize == '':
              query = query[:query.find('))')] + ')) FORMAT HEAD_RULE_EDB.'
#       graphFormat = query.find('FORMAT')
#       if graphFormat < 1:
#           query = query[:-1] + ' FORMAT TUPLE_RULE_TUPLE.'
            # if action == 'lingraph' and topk != '' and sSize != '':
            #   query = query[:query.find('))')] + ')) FORMAT TUPLE_ONLY. TOP ' + topk + ' SUMMARIZED BY LCA WITH SAMPLE(' + sSize + ').'
            if action == 'lingraph': #and topk == '' and sSize == '':
              query = query[:query.find('))')] + ')) FORMAT TUPLE_ONLY.'
#       graphFormat = query.find('FORMAT')
#       if graphFormat < 1:
#           query = query[:-1] + ' FORMAT TUPLE_ONLY.'
            # if action == 'provgame' and topk != '' and sSize != '':
            #    query = query[:query.find('))')] + ')). TOP ' + topk + ' SUMMARIZED BY LCA WITH SAMPLE(' + sSize + ').'
            if action == 'provgame': #and topk == '' and sSize == '':
               query = query[:query.find('))')] + ')).'

            query += summQuery
            queryhash = md5(query).hexdigest()
            imagefile = queryhash + '.svg'
            absImagepath = os.path.join(APP_STATIC, imagefile)
            
            if not(os.path.exists(absImagepath)):
                dotpath = os.path.join(APP_ROOT, 'tmp/pg.dot')
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
    # input db results
    # returncode, dblog = w.runInputDB(query)        
    dblog = ''
    inputDB = dblog
    dblog = conv.convert(dblog,full=False)
    rels = []
    if returncode == 0:
        lines=inputDB.split('\n')
	# collect relation names
	for eachel in lines:
	    if eachel.count('|') < 1 and eachel.count('-') < 1 and len(eachel) > 0:
		  rels += [eachel]
    # if query was successful then add to DB if we do not have it already
    if returncode == 0:
        existsAlready = db.session.query(ProvQuery).filter(ProvQuery.query == query and ProvQuery.graphOrResult == action).count()
        if existsAlready == 0:
            q = ProvQuery(query,imagefile,action)
            db.session.add(q)
            db.session.commit()
    allQueries = db.session.query(ProvQuery).all()
    return render_template('queryresult.html', query=query, gpromlog=gpromlog, dotlog=dotlog, imagefile=imagefile, returnedError=(returncode != 0), action=action, queryResult=queryResult, allQueries=allQueries, lines=lines, rels=rels, topk=topk, sSize=sSize, fPattern=fPattern, recall=recall, info=info)

if __name__ == '__main__':
    app.run(host='0.0.0.0')
