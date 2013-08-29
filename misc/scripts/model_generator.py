#! /usr/bin/env python
'''
This file automates the creation of *Model and *ModelManager sources.
'''
debug=0

import os
import sys

headers_path='../../Steel/include'
sources_path='../../Steel/src'
template_headers=[
    "templates/[['%(ClassName)s'%ref]]Model.h",
    "templates/[['%(ClassName)s'%ref]]ModelManager.h",
    ]
template_sources=[
    "templates/[['%(ClassName)s'%ref]]Model.cpp",
    "templates/[['%(ClassName)s'%ref]]ModelManager.cpp",
    ]

def write(lines, dst):
    """
    :param content: str
    :param dst: filepath
    """
    #import ipdb;ipdb.set_trace()
    dst%=globals()
    print 'writing %(dst)s'%locals()
    if debug:
        exit()
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
        #import ipdb;ipdb.set_trace()
        if -1 in [i,j]:
            break
        line=line[:i]+eval(line[i+2:j])+line[j+2:]
        # then redo the whole line since the eval mades i and j indefinite
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
        header_path, header_name=os.path.split(header)
        try:
            with open(header,'r') as file:
                write([populate(line,ref) for line in file.readlines()],
                os.path.join(headers_path,populate(header_name,ref)))
        except:
            print 'header:',header
            raise
    
    for source in template_sources:
        source_path,source_name=os.path.split(source)
        try:
            with open(source,'r') as file:
                write([populate(line,ref) for line in file.readlines()],
                os.path.join(sources_path,populate(source_name,ref)))
        except:
            print 'source:',source
            raise
        
    print '\nDont\'t forget to add those files to your Makefile:\ncd ../../build && cmake ..'
