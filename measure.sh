#!/bin/bash

time="1"     # one second
int=$1   # network interface

while true
        do
                txpkts_old="`cat /sys/class/net/$int/statistics/tx_packets`" # sent packets
                txbytes_old="`cat /sys/class/net/$int/statistics/tx_bytes`" # sent packets
                rxpkts_old="`cat /sys/class/net/$int/statistics/rx_packets`" # recv packets
                rxbytes_old="`cat /sys/class/net/$int/statistics/rx_bytes`" # sent packets
                        sleep $time
                txpkts_new="`cat /sys/class/net/$int/statistics/tx_packets`" # sent packets
                rxpkts_new="`cat /sys/class/net/$int/statistics/rx_packets`" # recv packets
                txbytes_new="`cat /sys/class/net/$int/statistics/tx_bytes`" # sent packets
                rxbytes_new="`cat /sys/class/net/$int/statistics/rx_bytes`" # sent packets
                txpkts="`expr $txpkts_new - $txpkts_old`"                    # evaluate expressions for sent packets
                rxpkts="`expr $rxpkts_new - $rxpkts_old`"                    # evaluate expressions for recv packets
                txbps="`expr $txbytes_new - $txbytes_old`"                    # evaluate expressions for sent packets
                rxbps="`expr $rxbytes_new - $rxbytes_old`"                    # evaluate expressions for recv packets
                        echo -en "\rtx $txpkts pkts/s, $txbps B/s - rx $rxpkts pkts/s, $rxbps B/s on interface $int"
        done
