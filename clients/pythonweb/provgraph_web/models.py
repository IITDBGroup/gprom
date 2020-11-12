from flask_sqlalchemy import SQLAlchemy
from provgraph_web import app

db = SQLAlchemy(app)

class ProvQuery(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    query = db.Column(db.String)
    imagepath = db.Column(db.String)
    graphOrResult = db.Column(db.String)
    
    def __init__(self, query, imagepath,graphOrResult):
        self.query = query
        self.imagepath = imagepath
        self.graphOrResult = graphOrResult

    def __repr__(self):
        return self.query + ":" + self.graphOrResult
