#!/bin/env python

import SOAPpy
import parca
import json
import subprocess
import datetime
import sys
import os
import os.path
from sqlalchemy import *
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
import tempfile
import time
import signal
import select
import logging

DATABASE = "mysql://root@localhost/parca_soap_test"
HOST = "localhost"
PORT = 8070
TEMPDIR = tempfile.gettempdir()
WORKERS = 4
WORKER_CHECK_TIMEOUT = 2
PIDFILE = None

for arg in sys.argv:
    if arg.startswith("--db="):
        DATABASE = arg[5:]
    elif arg.startswith("--host="):
        HOST = arg[7:]
    elif arg.startswith("--port="):
        PORT = int(arg[7:])
    elif arg.startswith("--tempdir="):
        TEMPDIR = arg[10:]
    elif arg.startswith("--pid="):
        PIDFILE = arg[6:]

log = None

if PIDFILE:
    f = open(PIDFILE,"w")
    f.write(str(os.getpid()))
    f.close()

if __name__=="__main__" and not "--worker" in sys.argv and not "--init" in sys.argv:
    logname = None
    for arg in sys.argv:
        if arg.startswith("--log="):
            logname = arg[6:]
    logging.basicConfig(filename=logname, format='%(asctime)-6s: %(levelname)s - %(message)s')
    log = logging.getLogger()
    log.setLevel(logging.INFO)
    log.info("Using database "+DATABASE)
    log.info("Running at "+HOST+" on port "+str(PORT))
    
    
engine = create_engine(DATABASE, echo=False)
Session = sessionmaker()
Session.configure(bind=engine)
session = Session()

Base = declarative_base()

ST_WAITING = 0
ST_WORKING = 1
ST_DONE = 2
ST_ERROR = 3

class ParcaJob(Base):
    __tablename__ = 'parca_jobs'

    id = Column(Integer, primary_key=True)
    # IP-address of owner client 
    ip = Column(String(20))
    # any strings
    title = Column(String(50))
    tag = Column(String(50))
    # date/time of job start and finish and status
    accept_datetime = Column(DateTime)
    start_datetime = Column(DateTime)
    finish_datetime = Column(DateTime)
    status = Column(Integer)
    error_string = Column(String(30))
    
    # source sequences
    sequence1 = Column(String(3000))
    sequence2 = Column(String(3000))
    # matrix to use
    matrix_name = Column(String(10))
    # Gap Elongation Penalty
    gep = Column(Float)
    # Maximum number of gaps
    gap_limit = Column(Integer)

    # Field for matching SW-alignment
    gop = Column(Float)

    # Field to match golden standard alignment
    golden_standard = Column(Text)

    # Alignments information stored in JSON format:
    # [ { "m", "g", "Acc", "Conf", "MinGOP", "MaxGOP" } ]
    alignment_infos = Column(Text)
    # Alignments stored in JSON format:
    # [ { "data" } ]
    alignments = Column(Text)

    def __init__(self, ip, title, seq1, seq2, tag="", limit=40, gep=1.0, matr="blosum62", gop=-1.0, gs=None):
        self.ip = ip
        self.sequence1 = seq1
        self.sequence2 = seq2
        self.title = title
        self.tag = tag
        self.gap_limit = limit
        self.gep = gep
        self.matrix_name = matr
        self.gop = gop
        if not gs is None:
            self.js = json.dumps(gs)
        else:
            self.js = ""
        self.accept_datetime = datetime.datetime.now()
        self.status = ST_WAITING
        self.error_string = ""
    
def process_job(job, session):
    if not parca.__dict__.has_key(job.matrix_name):
        job.error_string = "No matrix found: "+job.matrix_name
        return
    matr = parca.__dict__[job.matrix_name]
    parca.set_temporary_directory(TEMPDIR)
    pareto_als = parca.get_pareto_alignments(job.sequence1, job.sequence2, job.gep, matr, job.gap_limit)
    job.error_string = parca.get_last_error()
    res = []
    inf = []
    extrs = parca.get_primary_points(pareto_als)
    for al in pareto_als:
        d = []
        for a, b in al.data:
            d += [(a,b)]
        res += [{"data": d}]
        inf += [{"m": al.m, "g": al.g, "Acc": None, "Conf": None, "MinGOP": -1, "MaxGOP": -1, "Score": None, "Rating": None}]
    job.alignments = json.dumps(res)
    gops = parca.calculate_gops(pareto_als)
    for i in range(0,len(pareto_als)):
        inf[i]["MinGOP"], inf[i]["MaxGOP"] = gops[i]
    ratings = []
    for number, value in extrs:
        inf[number]["Score"] = value
    extrs.sort(key=lambda value: value[1], reverse=True)
    for i in range(0, len(extrs)):
        number, value = extrs[i]
        inf[number]["Rating"] = i+1
    job.alignment_infos = json.dumps(inf)
    job.finish_time = datetime.datetime.now()
    if len(job.error_string)>0:
        job.status = ST_ERROR
    else:
        job.status = ST_DONE
    session.commit()
    

def start_new_job(ip, title, seq1, seq2, tag="", limit=40, gep=1.0, matr="blosum62", gop=-1.0, gs=None):
    session = Session()
    job = ParcaJob(ip, title, seq1, seq2, tag, limit, gep, matr, gop, gs)
    session.add(job)
    session.commit()
    return job.id

def get_job_status(job_id):
    session = Session()
    query = session.query(ParcaJob.status).filter(ParcaJob.id==job_id)
    if query.count()==0:
        return "WRONG ID"
    status, = query.first()
    if status==ST_DONE: return "DONE"
    elif status==ST_WAITING: return "WAITING"
    elif status==ST_WORKING: return "WORKING"
    else: return "ERROR"
    
def get_error(job_id):
    session = Session()
    query = session.query(ParcaJob.error_string).filter(ParcaJob.id==job_id)
    if query.count()==0:
        return "WRONG ID"
    error_string, = query.first()
    return error_string

def list_jobs_for_ip(ip):
    session = Session()
    query = session.query(
        ParcaJob.id,
        ParcaJob.title,
        ParcaJob.tag,
        ParcaJob.accept_datetime,
        ParcaJob.status,
        ParcaJob.error_string
        ).filter(ParcaJob.ip==ip).all()
    res = []
    for id, title, tag, accept_datetime, status in query:
        if status==ST_ERROR:
            status = "ERROR"
        elif status==ST_DONE:
            status = "DONE"
        elif status==ST_WAITING:
            status = "WAITING"
        else:
            status = "WORKING"
        res += [ {
            "id": id,
            "title": title,
            "tag": tag,
            "start": accept_datetime,
            "status": status
            }]
    return res

def list_all_jobs():
    session = Session()
    query = session.query(
        ParcaJob.id,
        ParcaJob.title,
        ParcaJob.tag,
        ParcaJob.accept_datetime,
        ParcaJob.status,
        ParcaJob.error_string
        ).all()
    res = []
    for id, title, tag, accept_datetime, status in query:
        if status==ST_ERROR:
            status = "ERROR"
        elif status==ST_DONE:
            status = "DONE"
        elif status==ST_WAITING:
            status = "WAITING"
        else:
            status = "WORKING"
        res += [ {
            "id": id,
            "title": title,
            "tag": tag,
            "start": accept_datetime,
            "status": status
            }]
    return res

def get_alignments_count(id):
    session = Session()
    query = session.query(
        ParcaJob.status,
        ParcaJob.alignment_infos
        ).filter(ParcaJob.id==id)
    if query.count()==0:
        return 0
    status, infos = query.first()
    if status!=ST_DONE:
        return 0
    return len(json.loads(infos))

def get_alignment(id, no):
    session = Session()
    query = session.query(ParcaJob).filter(ParcaJob.id==id)
    if query.count()==0:
        return ""
    job = query.first()
    if job.status!=ST_DONE:
        return ""
    res = json.loads(job.alignments)
    inf = json.loads(job.alignment_infos)
    if no>=len(res):
        return ""
    minGOP = "n/a"
    maxGOP = "n/a"
    if not inf[no]["MinGOP"] is None:
        minGOP = str(inf[no]["MinGOP"])
    if not inf[no]["MaxGOP"] is None:
        maxGOP = str(inf[no]["MaxGOP"])
    rating = "not rated"
    score = "n/a"
    if not inf[no]["Score"] is None:
        rating = str(inf[no]["Rating"])
        score = str(inf[no]["Score"])
    al = res[no]["data"]
    s  = "; TITLE:             "+job.title+"\n"
    s += "; TAG:               "+job.tag+"\n"
    s += "; MATCH SCORE:       "+str(inf[no]["m"])+"\n"
    s += "; GAPS COUNT:        "+str(inf[no]["g"])+"\n"
    s += "; MINIMUM GOP VALUE: "+minGOP+"\n"
    s += "; MAXIMUM GOP VALUE: "+maxGOP+"\n"
    s += "; RATING:            "+rating+"\n"
    s += "; SCORE:             "+score+"\n"
    s += "; \n"
    
    matr = parca.__dict__[job.matrix_name]
    s += parca.alignment_to_string(job.sequence1, job.sequence2, res[no]["data"], None, matr)
    return s

def get_alignments_summary(id):
    session = Session()
    query = session.query(
        ParcaJob.title,
        ParcaJob.tag,
        ParcaJob.start_datetime,
        ParcaJob.sequence1,
        ParcaJob.sequence2,
        ParcaJob.status,
        ParcaJob.alignment_infos,
        ParcaJob.matrix_name,
        ParcaJob.gep,
        ParcaJob.gap_limit
        ).filter(ParcaJob.id==id)
    if query.count()==0:
        return None
    title, tag, start, s1, s2, status, inf, matr, gep ,limit = query.first()
    if status!=ST_DONE:
        return None
    inf = json.loads(inf)
    als = []
    for i in range(0, len(inf)):
        item = inf[i]
        item["no"] = i
        als += [item]
    result = {
        "title" : title,
        "tag" : tag,
        "sequence1": s1,
        "sequence2": s2,
        "matrix": matr,
        "gep": gep,
        "gap_limit": limit,
        "alignments": als
        }
    return result

PROCS = []
server = None
worker_job_id = None


def exit_handler_main(a,b):
    if PIDFILE and os.path.exists(PIDFILE):
        os.unlink(PIDFILE)
    log.info("Exiting...")
    log.info("Stopping server...")
    for proc in PROCS:
        log.info("Terminating worker "+str(proc.pid)+"...")
        proc.terminate()
    log.info("Bye!")

def exit_handler_worker(a,b):
    if not worker_job_id is None:
        session = Session()
        query = session.query(ParcaJob).filter(ParcaJob.id==worker_job_id).order_by(ParcaJob.accept_datetime).limit(1)
        if query.count()>0:
            job.status = ST_WAITING
            session.commit()
    sys.exit(0)

if __name__=="__main__" and "--init" in sys.argv:
    ParcaJob.metadata.create_all(engine)

elif __name__=="__main__" and "--worker" in sys.argv:
    signal.signal(signal.SIGTERM, exit_handler_worker)
    signal.signal(signal.SIGINT, exit_handler_worker)
    
    while True:
        session = Session()
        query = session.query(ParcaJob).filter(ParcaJob.status==ST_WAITING).order_by(ParcaJob.accept_datetime).limit(1)
        if query.count()==0:
            session.commit()
            time.sleep(WORKER_CHECK_TIMEOUT)
        else:
            job = query.first()
            job.status = ST_WORKING
            job.start_datetime = datetime.datetime.now()
            worker_job_id = job.id
            session.commit()
            process_job(job, session)
            worker_job_id = None
            print "done"
        del session
    
            
elif __name__=="__main__" and not "--worker" in sys.argv:
    script_path = os.path.abspath(__file__)
    python_path = sys.executable
    args = [python_path, script_path, "--worker"]
    for i in range(0, WORKERS):
        proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        log.info("Started worker: "+str(proc.pid))
        PROCS += [proc]
    signal.signal(signal.SIGTERM, exit_handler_main)
    signal.signal(signal.SIGINT, exit_handler_main)
    server = SOAPpy.ThreadingSOAPServer(addr=(HOST, PORT))
    server.registerFunction(start_new_job)
    server.registerFunction(get_job_status)
    server.registerFunction(list_jobs_for_ip)
    server.registerFunction(list_all_jobs)
    server.registerFunction(get_alignments_count)
    server.registerFunction(get_alignment)
    server.registerFunction(get_alignments_summary)
    server.registerFunction(get_error)
    try:
        server.serve_forever()
    except select.error:
        server.shutdown()
        if PIDFILE and os.path.exists(PIDFILE):
            os.unlink(PIDFILE)
  
  
    



    
