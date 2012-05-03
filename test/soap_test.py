import SOAPpy
import time

HOST = "localhost"
PORT = 8070
CHECK_TIMEOUT = 1

server = SOAPpy.SOAPProxy("http://"+HOST+":"+str(PORT)+"/")

S1 = u"ELTESTRTIPLDEAGGTTTLTARQFTNGQKIFVDTCT" + \
    "QCHLQGKTKTNNNVSLGLADLAGAEPRRDNVLALVEF" + \
    "LKNPKSYDGEDDYSELHPNISRPDIYPEMRNYTEDDI" + \
    "FDVAGYTLIAPKLDERWGGTIYF"
S2 = "EADLALGKAVFDGNCAACHAGGGNNVIPDHTLQKAAI" + \
    "EQFLDGGFNIEAIVYQIENGKGAMPAWDGRLDEDEIA" + \
    "GVAAYVYDQAAGNKW"

id = server.start_new_job("127.0.0.1", "test-title", S1, S2, "test-tag")

finished = False
while not finished:
    status = server.get_job_status(id)
    print "Status: ", status
    if status=="DONE" or status=="ERROR":
        break
    else:
        time.sleep(1)

if status=="ERROR":
    print server.get_error(id)

count = server.get_alignments_count(id)
for i in range(0,count):
    print server.get_alignment(id,i)



