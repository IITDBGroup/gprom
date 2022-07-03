import os

path = "/Users/liuziyu/gprom/lzy2"
files= os.listdir(path)
for file in files:
    if (file != ".DS_Store"):
        f = open(path+"/"+file)
        lines = f.read()
        print(lines)
        lib = 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/Users/liuziyu/ProjectLibrary/instantclient && '
        cmd = lib+'/Users/liuziyu/gprom/src/command_line/gprom -host ligeti.cs.iit.edu -db pdb1 -user TPCH_1GB -passwd IaDdpdr -port 1521 -log -loglevel 4 -sql '+lines+' -Pexecutor sql -Boracle.servicename TRUE -backend oracle'
        print(cmd)
        os.system(cmd)
        oldname=path+ "/" + file #设置新文件名
        newname=path + "/"+ file + "read"
        os.rename(oldname,newname)
        

#
    
