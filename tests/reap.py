# /usr/bin/env python
# vim:ft=python

# reap-sow helper script
# executed after the sow step
# AJ
import os
import re
import sys
import subprocess
import time

root_directory = os.path.dirname(os.path.realpath(__file__))


def sowList(first_out):
    print first_out
    """read the output from the 'sow' step to the list of files
    to run before the 'reap' step """
    find_cmd = re.compile("^(#|\s)\s+ psi4 -i (?P<infile>(?P<tag>[a-zA-Z]+)-[a-z0-9]+(-[a-z0-9]+)?\.in)\s+-o (?P<outfile>[a-zA-Z]+-[a-z0-9]+(-[a-z0-9]+)?\.out)")
    the_list = []
    with open(first_out,'r') as sow_out:
        for line in sow_out:
            #print line
            matchobj=find_cmd.match(line)
            if (matchobj):
                the_in = matchobj.group('infile')
                the_out = matchobj.group('outfile')
                the_tag=matchobj.group('tag')
                the_list.append((the_in,the_out))
    master_in=the_tag+"-master.in"
    master_out=the_tag+"-master.out"

    return the_list,master_in,master_out


def storeTests(testDir):
    """Store the tests in array"""
    testFile = os.path.join(testDir,"tests")
    retTests = []
    with open(testFile) as F:
        for line in F:
            retTests.append(line.strip())

    return retTests

def addTests(theMaster,theTests):
    """ Append the tests to the master file, it must be created first"""
    with open(theMaster,'a') as F:
        for test in theTests:
            F.write("{}\n".format(test))
    pass



def runFiles(psi4,theFileList,runingDir):
    """ run the input files generated by the first input """
    for infile,outfile in theFileList:
        thisInFile=os.path.join(runingDir,infile.strip())
        thisOutFile=os.path.join(runingDir,outfile.strip())
        cmd = [psi4,"-i",thisInFile,"-o",thisOutFile]
        retcode = subprocess.call(cmd)
        if (retcode is not None):
            sys.stdout.write('Multi-invocation %s exited with status %i\n' % (cmd, retcode))


def runMaster(psi4, inMasterFile, outMasterFile,logfile,psi4datadir):
    """ run the master file and record the output in the logfile,
    the same way that runtest.py does it."""
    cmd = [psi4,"-i",inMasterFile,"-o",outMasterFile,"-l",psi4datadir]
    try:
        loghandle = open(logfile, 'a')
    except IOError as e:
        print ("""%s can't write to %s: %s""" %(__name__,logfile, e))
        sys.exit(1)
    try:
        retcode = subprocess.Popen(cmd, bufsize=0, stdout=subprocess.PIPE, universal_newlines=True)
    except OSError as e:
        sys.stderr.write('Command %s execution failed: %s\n' % cmd, e.strerror)
        sys.exit(1)
    p4out = ''
    while True:
        data = retcode.stdout.readline()
        if not data:
            break
        sys.stdout.write(data) # screen
        loghandle.write(data) # file
        loghandle.flush()
        p4out += data # string
    loghandle.close()
    while True:
        retcode.poll()
        exstat = retcode.returncode
        if exstat is not None:
            return exstat
        time.sleep(0.1)


def main(first_input,first_output,logfile, psi4,psi4datadir):
    # check the psi4 path is there and is executable
    input_dir = os.path.dirname(first_input)
    output_dir = os.path.dirname(first_output)
    if os.path.isfile(psi4) and os.access(psi4,os.X_OK):
        # get the list of intermediate input files
        # files list and master file are not abolute paths just names
        files_list,master_in,master_out= sowList(first_output)
        # get the commands from tests
        tests=storeTests(input_dir)
        # set the "reapmode" master file full path
        reap_master_in=os.path.join(output_dir,master_in.strip())
        reap_master_out=os.path.join(output_dir,master_out.strip())
        # append the tests
        addTests(reap_master_in,tests)
        # run intermediates
        runFiles(psi4,files_list,output_dir)
        # run the master
        runMaster(psi4,reap_master_in,reap_master_out,logfile,psi4datadir)
        # sucess
        sys.exit(0)
    else:
        print("""check psi4 path  %s is not executable, or was not found""" % (psi4))
        sys.exit(1)



if __name__=='__main__':
    main(*sys.argv[1:])







