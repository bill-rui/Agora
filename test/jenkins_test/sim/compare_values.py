'''
compare the values from the BER value file to a threshold
requires the first argument being the threshold to compare against
raise ValueError("BER threshold exceeded") when test fails
'''

from optparse import OptionParser
parser: OptionParser = OptionParser(usage="%prog [BER threshold]", 
                        description="get the BER values from an output file to compare to a threshold")
(option, args) = parser.parse_args()

file = open("./test/jenkins_test/sim/BER_values.txt", "r")
values = []
for line in file:
    words = line.split(" ")
    value = ""
    found = False
    for letter in words[6]:
        if letter == "(":
            found = True
        elif letter == ")":
            break
        elif found == True:
            value += letter
        
    values.append(value)

threshold = args[0]
success = True
for i in range(len(values)):
    value = values[i]
    value = float(value)
    if value > float(threshold):
        print("[WARNING] BER threshold of UE " + str(i) + " exceeded, test failed")
        success = False
    else:
        pass

if not success:
    raise ValueError("BER threshold execeeded")

file.close()