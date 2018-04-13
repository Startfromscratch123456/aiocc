#!/usr/bin/env python



from .aiocc_logging import *
import glob
import os
import re
from typing import Dict


protocol_ver = 1


def extract_ack_ewma_from_import(import_data: str) -> float:
    return float(re.search('(?<=ack_ewma: )[0-9.]+', import_data).group(0))


def extract_sent_ewma_from_import(import_data: str) -> float:
    return float(re.search('(?<=sent_ewma: )[0-9.]+', import_data).group(0))


def extract_rtt_ratio100_from_import(import_data: str) -> float:
    return float(re.search('(?<=rtt_ratio100: )[0-9.]+', import_data).group(0))


def extract_read_bandwidth_from_import(import_data: str) -> float:
    return float(re.search('(?<=read_throughput: )[0-9.]+', import_data).group(0))


def extract_write_bandwidth_from_import(import_data: str) -> float:
    return float(re.search('(?<=write_throughput: )[0-9.]+', import_data).group(0))


def gen_rule(rule_template: str, kv: Dict[str, float]) -> str:
    result = rule_template
    for key, val in kv.items():
        result = result.replace('{{{{ {key} }}}}'.format(key=key), str(val))
    return result


def read_proc_file(filename: str) -> float:
    """Read one number from 'osc_path/filename'
    """
    with open(filename, 'rt') as procfile:
        try:
            # Don't use readline() because it can cause out-of-memory error when reading a bizarre procfile
            return float(procfile.read(100))
        except (OSError, ValueError) as e:
            logging.error('{type}: {msg}'.format(type=type(e).__name__, msg=str(e)))
            return 0


def set_procfs_osc(filename: str, data: int, num_osc: int) -> None:
    control_files = glob.glob('/proc/fs/lustre/osc/*/' + filename)
    assert len(control_files) == num_osc

    for cf in control_files:
        with open(cf, 'w') as fh:
            fh.write(str(data))


def set_mrif(mrif: int, num_osc: int) -> None:
    set_procfs_osc('max_rpcs_in_flight', mrif, num_osc)
    # set_procfs_osc('max_dirty_mb', min(256, 4 * mrif), num_osc)


def set_rule(rule: str, num_osc: int) -> None:
    rule_files = glob.glob('/proc/fs/lustre/osc/*/qos_rules')
    assert len(rule_files) == num_osc
    for rule_file in rule_files:
        with open(rule_file, 'w') as fh:
            fh.write(rule)
