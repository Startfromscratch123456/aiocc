# description:   merge qos rules which gathered from client nodes during  previous phase,
#                write the merged file to output file
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-10
#
import sys
import os

qos_rule_files_dir = sys.argv[1]
merged_rule_output_file = sys.argv[2]
rule_file_suffix = ".rule"

rules = []
global rule_no
global row_index
is_first_file = True
qos_rule_files = os.listdir(qos_rule_files_dir)

for qos_rule_file_name in qos_rule_files:
    if not qos_rule_file_name.endswith(rule_file_suffix):
        print("skip file %s", qos_rule_file_name)
        continue
    with open(qos_rule_files_dir + os.sep + qos_rule_file_name, 'r') as qos_rule_file:
        qos_rule_file_content = qos_rule_file.readlines()
        rule_no = 0
        row_index = 0
        for line_str in qos_rule_file_content:
            rule_parameter = line_str.split(',')
            if rule_no == 0:
                rule_no = int(rule_parameter[0])
                mrif_update_rate = int(rule_parameter[1])
                continue
            if is_first_file:
                new_rule = dict(ack_ewma_lower=int(rule_parameter[0]),
                                ack_ewma_upper=int(rule_parameter[1]),
                                send_ewma_lower=int(rule_parameter[2]),
                                send_ewma_upper=int(rule_parameter[3]),
                                rtt_ratio100_lower=int(rule_parameter[4]),
                                rtt_ratio100_upper=int(rule_parameter[5]),
                                m100=int(rule_parameter[6]),
                                b100=int(rule_parameter[7]),
                                tau=int(rule_parameter[8]),
                                used_times=int(rule_parameter[9]),
                                ack_ewma_avg=int(rule_parameter[10]),
                                send_ewma_avg=int(rule_parameter[11]),
                                rtt_ratio100_avg=int(rule_parameter[12]))
                rules.append(new_rule)
            else:
                if (rules[row_index]["ack_ewma_lower"] != int(rule_parameter[0]) or
                            rules[row_index]["ack_ewma_upper"] != int(rule_parameter[1]) or
                            rules[row_index]["send_ewma_lower"] != int(rule_parameter[2]) or
                            rules[row_index]["send_ewma_upper"] != int(rule_parameter[3]) or
                            rules[row_index]["rtt_ratio100_lower"] != int(rule_parameter[4]) or
                            rules[row_index]["rtt_ratio100_upper"] != int(rule_parameter[5]) or
                            rules[row_index]["m100"] != int(rule_parameter[6]) or
                            rules[row_index]["b100"] != int(rule_parameter[7]) or
                            rules[row_index]["tau"] != int(rule_parameter[8])):
                    print(
                        "Rule file %s doesn't agree with previous rule files, maybe it's corrupted." % qos_rule_file_name,
                        file=sys.stderr)
                    exit(3)

                # na+nb
                rules[row_index]["used_times"] += int(rule_parameter[9])
                if rules[row_index]["used_times"] != 0:
                    nx = rules[row_index]["used_times"]
                    nb = int(rule_parameter[9])
                    xa = rules[row_index]["ack_ewma_avg"]
                    xb = int(rule_parameter[10])
                    delta = xb - xa
                    rules[row_index]["ack_ewma_avg"] = xa + delta * nb / nx

                    nb = int(rule_parameter[9])
                    xa = rules[row_index]["send_ewma_avg"]
                    xb = int(rule_parameter[11])
                    delta = xb - xa
                    rules[row_index]["send_ewma_avg"] = xa + delta * nb / nx

                    nb = int(rule_parameter[9])
                    xa = rules[row_index]["rtt_ratio100_avg"]
                    xb = int(rule_parameter[12])
                    delta = xb - xa
                    rules[row_index]["rtt_ratio100_avg"] = xa + delta * nb / nx
            row_index += 1

    is_first_file = False

output_file = open(merged_rule_output_file, 'w')
print("%d,%d" % (rule_no, mrif_update_rate), file=output_file)
row_id = 0
for rule in rules:
    print('%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d' % (
        rule["ack_ewma_lower"], rule["ack_ewma_upper"],
        rule["send_ewma_lower"], rule["send_ewma_upper"],
        rule["rtt_ratio100_lower"], rule["rtt_ratio100_upper"],
        rule["m100"], rule["b100"], rule["tau"], rule["used_times"],
        rule["ack_ewma_avg"], rule["send_ewma_avg"],
        rule["rtt_ratio100_avg"]), file=output_file)
