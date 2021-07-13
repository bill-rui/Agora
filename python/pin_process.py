"""
pin processes of a user to a given set of cores
"""
import subprocess as s
from optparse import OptionParser

def pin_PID_to_cores(cores: str, pid: str):
    """
    pin given pid to set of cores
    :param cores: set of cores
    :param pid: pid of process
    """
    process = s.run(["taskset", "-cap", cores, pid], stdout=s.PIPE, universal_newlines=True)

def find_PID(username: str) -> list:
    """
    find PID of all processes from given useranme
    :param username: username to be searched
    :return: a list of PID in string
    """
    process: s.CompletedProcess = s.run(["ps", "aux"], 
                                            stdout=s.PIPE, universal_newlines=True)

    results:str = process.stdout
    lines:list = results.split("\n")
    lines.pop(0)
    PID_list = []

    for line in lines:
        current_line = line.split(" ")
        if current_line[0] == username:
            current_line.pop(0)
            for word in current_line:
                if word != "":
                    PID_list.append(word)
                    break

    return PID_list


parser: OptionParser = OptionParser(usage="%prog [username] [cores]", 
                        description="pin all processes from user to a set of cores")
(option, args) = parser.parse_args()

PID_list = find_PID(args[0])
for pid in PID_list:
    pin_PID_to_cores(args[1], pid)
    
