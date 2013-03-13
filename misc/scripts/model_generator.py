'''
This file automates the creation of *Model and *ModelManager sources.
'''

import os
import sys

headers_path='../../Steel/include'
sources_path='../../Steel/src'
template_headers=[
    "tempaltes/[['%(ClassName)s'%ref]]Model.h",
    "tempaltes/[['%(ClassName)s'%ref]]ModelManager.h",
    ]
template_sources=[
    "tempaltes/[['%(ClassName)s'%ref]]Model.cpp",
    "tempaltes/[['%(ClassName)s'%ref]]ModelManager.cpp",
    ]

def write(lines,dst):
    """
    :param content: str
    :param dst: filepath
    """
    #import ipdb;ipdb.set_trace()
    print 'writing %(dst)s'%locals()
    with open(dst,'w') as file:
        file.write(''.join(lines))
    
def populate(line, ref):
    """
    :param lines: list of string, content of the file.
    :param ref: dict for string replacements
    """
    line=str(line)
    i=j=0
    while True:
        i=line.find('[[',i)
        j=line.find(']]',i)
        if -1 in [i,j]:
            break
        line=line[:i]+eval(line[i+2:j])+line[j+2:]
        i=j+2
    return line
    

if __name__=='__main__':
    if len(sys.argv)<3:
        print 'this script takes a class name argument and a type name argument'
        print 'example: python model_generator.py Test TEST'
        print 'generates files for TestModel with type MT_TEST'
        exit()
    class_name=sys.argv[1]
    type_name=sys.argv[2]
    ref={
        'ClassName':class_name,
        'TypeName':type_name,
    }
    
    for header in template_headers:
        try:
            with open(header,'r') as file:
                write([populate(line,ref) for line in file.readlines()],
                    os.path.join(headers_path,populate(header,ref)))
        except:
            print 'header:',header
            raise
            
    for source in template_sources:
        try:
            with open(source,'r') as file:
                write([populate(line,ref) for line in file.readlines()],
                    os.path.join(sources_path,populate(source,ref)))
        except:
            print 'source:',source
            raise
