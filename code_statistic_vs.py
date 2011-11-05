import __future__
import sys, os
import os.path

def get_file_name(line_trip):
    lcomma = line_trip.find("\"")
    rcomma = line_trip.rfind("\"")
    file_name = ""
    if lcomma != -1 and rcomma != -1:
        file_name = line_trip[lcomma+1 : rcomma]
    return file_name

def get_file_ext(file_name):
    ext = ""
    if len(file_name) == 0:
        return ext
    index = file_name.rfind(".")
    if index != -1:
        ext = file_name[index:]
    return ext

def get_file_depth(file_name_split):
    count = 0
    if file_name_split[0] != ".." and file_name_split[0] != ".":
        return -1
    
    for x in file_name_split:
        if x == "..":
            count += 1
        else:
            break
    return count
            
def get_file_path(prj_name, file_name):
        prj_name_split = prj_name.replace("\\", "/").split('/')
        file_name_split = file_name.replace("\\", "/").split('/')

        count = get_file_depth(file_name_split)
        if count == -1:
            return file_name
        else:
            prj_depth = -1 * (count + 1)
            file_depth = 0
            if count == 0:
                file_depth = 1
            else:
                file_depth = count

            file_path = ""
            for x in prj_name_split[:prj_depth]:
                file_path += x + '/'
            for x in file_name_split[file_depth:]:
                if x == file_name_split[len(file_name_split) - 1]:
                    file_path += x
                else:
                    file_path += x + '/'
            return file_path

def stat_file(file_name, stat):
    f = open(file_name)
    lines = f.readlines()
    is_multiline_comment = False
    for l in lines:
        stripped_l = l.strip()
        stat["total lines"] += 1
        if not is_multiline_comment:
            if stripped_l.startswith("//"):
                stat["single line comments"] += 1
                continue
            if stripped_l.find("/*") != -1:
                is_multiline_comment = True
                stat["multi line comments"] += 1
            else:
                if stripped_l == "":
                    stat["null lines"] += 1
                elif stripped_l == "{" or stripped_l == "}":
                    stat["empty lines"] += 1
                else:
                    stat["code lines"] += 1
        else:
            if stripped_l.find("*/") != -1:
                is_multiline_comment = False
            stat["multi line comments"] += 1


    f.close()
    return file_name

def stat_prjvc8(prj_name, ext_filter):
    f = open(prj_name, "r")
    lines = f.readlines()
    into_files = False
    into_file  = False

    stat = dict([(x,0) for x in ext_filter])
    stat["total lines"] = 0
    stat["single line comments"] = 0
    stat["multi line comments"] = 0
    stat["null lines"] = 0
    stat["empty lines"] = 0
    stat["code lines"] = 0
    for line in lines:
        line_trip = line.strip()
        if line_trip == "<Files>":
            print "into Files"
            into_files = True
        elif into_files:
            if line_trip == "<File":
                into_file = True
            elif line_trip == "</File>":
                into_file = False
            elif into_file:
                file_name = get_file_name(line_trip)
                ext = get_file_ext(file_name)
                if ext in ext_filter:
                    stat[ext] += 1
		    stat_file(get_file_path(prj_name, file_name), stat)      
            if line_trip == "</Files>":
                into_files = False
                print "out Files"
                f.close()
                return stat

def stat_prjvc6(prj_name, ext_filter):
    f = open(prj_name, "r")
    lines = f.readlines()
    into_files = False

    stat = dict([(x,0) for x in ext_filter])
    stat["total lines"] = 0
    stat["single line comments"] = 0
    stat["multi line comments"] = 0
    stat["null lines"] = 0
    stat["empty lines"] = 0
    stat["code lines"] = 0
    for line in lines:
        line_trip = line.strip()
        if line_trip == "# Begin Target":
            print "into Files"
            into_files = True
        elif into_files:
            if line_trip.find("SOURCE=") != -1:
                line_exp = line_trip.split('=')
                ext = get_file_ext(line_exp[1])
                if ext in ext_filter:
                    stat[ext] += 1
		    stat_file(get_file_path(prj_name, line_exp[1]), stat)      
            if line_trip == "# End Target":
                into_files = False
                print "out Files"
                f.close()
                return stat

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("error: paramters are not enough.")
        print("use this script as:")
        print(" code_statistic_vs.py vsprj_name [\".ext;.fuck\"]")
    else:
        if len(sys.argv) == 3:
            ext = sys.argv[2]
        else:
            ext = ".h;.cpp"

        stat = stat_prjvc6(sys.argv[1], set(ext.split(";")))
        
        print("code statistic prj: %s" % sys.argv[1])
        print("with extensions: %s" % ext)
        print("result:")

        for key in set(ext.split(";")):
            print ("%s: %d" % (key, stat[key]))

        key_val = ["total lines", "code lines", "single line comments", "multi line comments",
                    "null lines", "empty lines"]
               
        for key in key_val:
            print ("%s: %d" % (key, stat[key]))
