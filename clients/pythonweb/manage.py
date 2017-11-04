#!/usr/bin/env python
from flask_script import Manager, Shell, Server
from provgraph_web import app

manager = Manager(app)
manager.add_command("runserver", Server())
manager.add_command("shell", Shell())

@manager.command
def createdb():
    from provgraph_web.models import db
    db.create_all()

@manager.command
def startserver():
    from provgraph_web import app
    app.run(host='0.0.0.0')
    
manager.run()
