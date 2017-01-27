from flask import Flask, render_template, request, redirect, url_for, abort, session
from gprom_wrapper import GProMWrapper
from hashlib import md5
from ansi2html import Ansi2HTMLConverter
import markdown

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/querysubmit', methods=['POST'])
def querysubmit():
    session['query'] = request.form['query']
    if request.form.has_key('gengraph'):
        session['action'] = 'graph'
    else:
        session['action'] = 'run'
    return redirect(url_for('showgraph'))

@app.route('/showgraph')
def showgraph():
    if not 'query' in session:
        return abort(403)
    w = GProMWrapper()
    query = session['query']
    action = session['action']
    conv = Ansi2HTMLConverter()
    # generate a graph
    if action == 'graph':
        queryResult=''
        queryhash = md5(query).hexdigest()
        imagefile = queryhash + '.svg'
        absImagepath = 'static/' + imagefile
        returncode, gpromlog, dotlog = w.generateProvGraph(query, absImagepath, 'tmp/pg.dot')
        gpromlog = conv.convert(gpromlog,full=False)
        dotlog = conv.convert(dotlog,full=False)
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
    return render_template('queryresult.html', query=session['query'], gpromlog=gpromlog, dotlog=dotlog, imagefile=imagefile, returnedError=(returncode != 0), action=action, queryResult=queryResult)

if __name__ == '__main__':
    app.run()
