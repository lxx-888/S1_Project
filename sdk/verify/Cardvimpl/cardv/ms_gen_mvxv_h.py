#!/usr/bin/python
#V2
import re, fnmatch, os, sys, subprocess, argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate MVXXVM .h file for P4 system')

    parser.add_argument("file", help="Output File")
    parser.add_argument("--changelist", help="Change List String", default="#0000000")
    parser.add_argument("--lib_type", help="Library Type",default="#")
    parser.add_argument("--chip_id", help="Chip ID",default="##")
    parser.add_argument("--comp_id", help="COMP_ID",default="#####")
#    parser.add_argument("--p4_change", help="Write the $Change for P4",action="store_true")

    parser.add_argument("--branch", help="branch")
    args=parser.parse_args()

    #print args
    
    if args.file is None :
        print 'ERROR: must specify output header file!!'
        sys.exit(-1)

    version_file=open(args.file,'w')
    
#    if args.p4_change is not None:
#        version_file.write("// $Change: %s $\n" % args.changelist[1:])

    while len(args.comp_id) < 5 :
        args.comp_id += '#'
        
    while len(args.chip_id) < 2 :
        args.chip_id += '#'
        
    while len(args.changelist) < 8 :
        args.changelist += '#'

    while len(args.lib_type) < 2 :
        args.lib_type += '#'

    ext_val=''
    ext_str=''
    if args.branch is not None :
        ext_val += '[BR:'
        ext_val += args.branch
        ext_val += ']'

    if len(ext_val) > 0 :
            ext_str += ext_val

#    version_file.write('#define MVXV_HEAD_VER   ' + str(list('2')).replace('[','{').replace(']','}') + '\n')
#    version_file.write('#define MVXV_LIB_TYPE   ' + str(list(args.lib_type)).replace('[','{').replace(']','}') + '\n')
#    version_file.write('#define MVXV_CHIP_ID    ' + str(list(args.chip_id)).replace('[','{').replace(']','}') + '\n')
#    version_file.write('#define MVXV_CHANGELIST ' + str(list(args.changelist)).replace('[','{').replace(']','}') + '\n')
#    version_file.write('#define MVXV_COMP_ID    ' + str(list(args.comp_id)).replace('[','{').replace(']','}') + '\n')
    version_file.write('#define MVXV_HEAD_VER   "' + '2'+'"\n')
    version_file.write('#define MVXV_LIB_TYPE   "' + args.lib_type + '"\n')
    version_file.write('#define MVXV_CHIP_ID    "' + args.chip_id + '"\n')
    version_file.write('#define MVXV_CHANGELIST "' + args.changelist + '"\n')
    version_file.write('#define MVXV_COMP_ID    "' + args.comp_id + '"\n')
    if len(ext_str) > 0 :
        #ext_str=str(list(ext_str))
        #ext_str=ext_str[1:(len(ext_str)-1)]
        version_file.write('#define MVXV_EXT        "' + ext_str + '"\n')
    version_file.write("\n")
    version_file.close()
