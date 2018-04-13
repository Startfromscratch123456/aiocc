package collect

import (
	"time"
	"strconv"
	"strings"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/cpu"
	psutilnet "github.com/shirou/gopsutil/net"
	"github.com/shirou/gopsutil/process"
	"../util"
	"os"
	"bufio"
	"io"
	"errors"
	"io/ioutil"
	"unicode"
	"../conf"
)

/* 1.避免需要向上取整问题（某些指标计量不向上取整可能你最终结果为0）;2.自动处理a<0的问题
  a在什么时候小于0：https://www.kernel.org/doc/Documentation/iostats.txt
  "all others only increase (unless they overflow and wrap)"
*/
func DoValidFloatDivide(a float64, b float64) float64 {
	if a <= 0 || b <= 0 {
		return 0
	}
	return a / b
}

type DiskCollect struct {
	lastDiskIOCountersStat map[string]disk.IOCountersStat
	lastTimeStamp          int64
}

func (diskCocllect *DiskCollect) DiskCollectInit() (error) {
	curDiskIOCountersStat, err := disk.IOCounters()
	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}
	diskCocllect.lastDiskIOCountersStat = curDiskIOCountersStat
	diskCocllect.lastTimeStamp = time.Now().Unix()
	return nil
}

/*判断磁盘只读*/
func (diskCocllect *DiskCollect) readOnly(path string) (bool) {
	fileName := path + ".READONLY"
	if _, err := os.Stat(fileName); !os.IsNotExist(err) {
		return false
	}
	file, err := os.OpenFile(fileName, os.O_CREATE|os.O_RDWR, 0666)
	if err != nil {
		return true
	}
	str := "00000000000000000"
	n, err := file.WriteString(str)
	if len(str) != n || err != nil {
		return true
	}
	os.Remove(fileName)
	return false
}

/**
 * 磁盘名称<-->磁盘信息<-->[属性名:属性值]
 */
func (diskCocllect *DiskCollect) getDiskIO() (map[string]map[string]string, error) {
	diskIOInfoRet := make(map[string]map[string]string)
	curDiskIOCountersStat, err := disk.IOCounters()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return diskIOInfoRet, err
	}

	curTime := time.Now().Unix()
	timeDiff := float64(curTime - diskCocllect.lastTimeStamp)

	for key, cur := range curDiskIOCountersStat {
		keyBytes := []byte(key) //解决磁盘以及其分区同时出现的问题 disk.IOCounters本质是读取/proc/partitions

		if unicode.IsDigit(rune(keyBytes[len(keyBytes)-1])) == false {
			continue
		}

		if last, ok := diskCocllect.lastDiskIOCountersStat[key]; ok {
			readTime := float64(cur.ReadTime - last.ReadTime)
			writeTime := float64(cur.WriteTime - last.WriteTime)

			readCount := float64(cur.ReadCount - last.ReadCount)
			writeCount := float64(cur.WriteCount - last.WriteCount)

			diskIOVar := make(map[string]string)
			diskIOVar["diskReadIO"] = strconv.FormatFloat(DoValidFloatDivide(float64(cur.ReadBytes-last.ReadBytes), timeDiff), 'f', -1, 64)
			diskIOVar["diskWriteIO"] = strconv.FormatFloat(DoValidFloatDivide(float64(cur.WriteBytes-last.WriteBytes), timeDiff), 'f', -1, 64)
			//平均每次设备I/O操作的等待时间
			diskIOVar["diskRWAwait"] = strconv.FormatFloat(DoValidFloatDivide(readTime+writeTime, readCount+writeCount), 'f', -1, 64)
			diskIOVar["diskReadAwait"] = strconv.FormatFloat(DoValidFloatDivide(readTime, readCount), 'f', -1, 64)
			diskIOVar["diskWriteAwait"] = strconv.FormatFloat(DoValidFloatDivide(writeTime, writeCount), 'f', -1, 64)
			diskIOVar["diskReadIops"] = strconv.FormatFloat(DoValidFloatDivide(readCount, timeDiff), 'f', -1, 64)
			diskIOVar["diskWriteIops"] = strconv.FormatFloat(DoValidFloatDivide(writeCount, timeDiff), 'f', -1, 64)
			diskIOVar["diskIopsInProgress"] = strconv.FormatFloat(DoValidFloatDivide(float64(cur.IopsInProgress-last.IopsInProgress), timeDiff), 'f', -1, 64) //Number of actual I/O requestscurrently in flight.
			diskIOVar["diskWeightedIO"] = strconv.FormatFloat(DoValidFloatDivide(float64(cur.WeightedIO-last.WeightedIO), timeDiff), 'f', -1, 64)
			diskIOVar["diskSerialNumber"] = last.SerialNumber
			diskIOInfoRet[key] = diskIOVar
		}

	}

	diskCocllect.lastDiskIOCountersStat = curDiskIOCountersStat
	diskCocllect.lastTimeStamp = curTime

	return diskIOInfoRet, nil
}

/**
 * 磁盘名称<-->磁盘信息<-->[属性名:属性值]
 */
func (diskCocllect *DiskCollect) GetPartitionInfo() (map[string]map[string]string, error) {
	//判断是否为只读
	diskPartitionInfoRet := make(map[string]map[string]string)
	//false:只返回物理设备信息(hard disks/cd-rom drives/USB keys),不包括其它信息(内存分区如/dev/shm)
	partitionStats, err := disk.Partitions(false)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return diskPartitionInfoRet, err
	}
	diskIO, err := diskCocllect.getDiskIO()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return diskIO, err
	}

	for k, v := range diskIO {
		diskPartitionInfoRet[k] = v
	}

	for _, partitionStat := range partitionStats {

		name := partitionStat.Device[strings.LastIndex(partitionStat.Device, "/")+1:]

		if _, ok := diskPartitionInfoRet[name]; !ok {
			diskPartitionInfoRet[name] = make(map[string]string)
		}

		diskUsage, _ := disk.Usage(partitionStat.Mountpoint)
		diskPartitionInfoRet[name]["diskFree"] = strconv.FormatUint(diskUsage.Free, 10)
		diskPartitionInfoRet[name]["diskTotal"] = strconv.FormatUint(diskUsage.Total, 10)
		diskPartitionInfoRet[name]["diskInodesFree"] = strconv.FormatUint(diskUsage.InodesFree, 10)
		diskPartitionInfoRet[name]["diskInodesTotal"] = strconv.FormatUint(diskUsage.InodesTotal, 10)
		diskPartitionInfoRet[name]["diskType"] = diskUsage.Fstype
		diskPartitionInfoRet[name]["diskDeviceName"] = partitionStat.Device
		diskPartitionInfoRet[name]["diskReadOnly"] = strconv.FormatBool(diskCocllect.readOnly(partitionStat.Mountpoint)) //只读 读写
	} //for

	return diskPartitionInfoRet, nil
}

type MemoryCollect struct {
	lastSwapMemoryStat map[string]uint64
	lastTimeStamp      int64
}

func (memoryCollect *MemoryCollect) MemoryCollectInit() (error) {
	swapMemStat, err := mem.SwapMemory()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	memoryCollect.lastSwapMemoryStat = make(map[string]uint64)
	memoryCollect.lastSwapMemoryStat["sin"] = swapMemStat.Sin
	memoryCollect.lastSwapMemoryStat["sout"] = swapMemStat.Sout
	memoryCollect.lastTimeStamp = time.Now().Unix()

	return nil
}

func (memoryCollect *MemoryCollect) GetSwapMemoryInfo() (map[string]string, error) {
	swapMemoryInfoRet := make(map[string]string)
	swapMemoryInfo, err := mem.SwapMemory()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return swapMemoryInfoRet, err
	}

	swapMemoryInfoRet["swapMemoryTotal"] = strconv.FormatUint(swapMemoryInfo.Total, 10)
	swapMemoryInfoRet["swapMemoryFree"] = strconv.FormatUint(swapMemoryInfo.Free, 10)
	swapMemoryInfoRet["swapMemoryTotal"] = strconv.FormatUint(swapMemoryInfo.Total, 10)

	curTime := time.Now().Unix()
	timeDiff := float64(curTime - memoryCollect.lastTimeStamp)
	memoryCollect.lastSwapMemoryStat["sin"] = swapMemoryInfo.Sin
	memoryCollect.lastSwapMemoryStat["out"] = swapMemoryInfo.Sout
	memoryCollect.lastTimeStamp = curTime

	swapMemoryInfoRet["swapMemorySin"] = strconv.FormatFloat(DoValidFloatDivide(float64(swapMemoryInfo.Sin-memoryCollect.lastSwapMemoryStat["sin"]), timeDiff), 'f', -1, 64)
	swapMemoryInfoRet["swapMemorySout"] = strconv.FormatFloat(DoValidFloatDivide(float64(swapMemoryInfo.Sout-memoryCollect.lastSwapMemoryStat["out"]), timeDiff), 'f', -1, 64)

	return swapMemoryInfoRet, nil
}

func (memCollect *MemoryCollect) GetMemoryInfo() (map[string]string, error) {
	memoryInfoRet := make(map[string]string)
	memoryInfo, err := mem.VirtualMemory()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return memoryInfoRet, err
	}

	memoryInfoRet["memoryTotal"] = strconv.FormatUint(memoryInfo.Total, 10)
	memoryInfoRet["memoryFree"] = strconv.FormatUint(memoryInfo.Free, 10)
	memoryInfoRet["memoryAvailable"] = strconv.FormatUint(memoryInfo.Available, 10)
	memoryInfoRet["memoryUsed"] = strconv.FormatUint(memoryInfo.Used, 10)
	memoryInfoRet["memoryInactive"] = strconv.FormatUint(memoryInfo.Inactive, 10)
	memoryInfoRet["memoryActive"] = strconv.FormatUint(memoryInfo.Active, 10)
	memoryInfoRet["memoryBuffers"] = strconv.FormatUint(memoryInfo.Buffers, 10)
	memoryInfoRet["memoryCached"] = strconv.FormatUint(memoryInfo.Cached, 10)
	memoryInfoRet["memoryDirty"] = strconv.FormatUint(memoryInfo.Dirty, 10)
	memoryInfoRet["memoryShared"] = strconv.FormatUint(memoryInfo.Shared, 10)
	memoryInfoRet["memorySlab"] = strconv.FormatUint(memoryInfo.Slab, 10)
	memoryInfoRet["memorySwapCached"] = strconv.FormatUint(memoryInfo.SwapCached, 10)
	memoryInfoRet["memoryWired"] = strconv.FormatUint(memoryInfo.Wired, 10)
	memoryInfoRet["memoryWriteback"] = strconv.FormatUint(memoryInfo.Writeback, 10)
	memoryInfoRet["memoryWritebackTmp"] = strconv.FormatUint(memoryInfo.WritebackTmp, 10)

	return memoryInfoRet, nil
}

type CpuCollect struct {
	lastCpuTimesStat []cpu.TimesStat
	lastTimeStamp    int64
}

func (cpuCollect *CpuCollect) CpuCollectInit() (error) {
	cpuCollect.lastTimeStamp = time.Now().Unix()

	cpusTimesStatCur, err := cpu.Times(true)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	cpuTimesStatCurTotal, err := cpu.Times(false)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	cpuCollect.lastCpuTimesStat = append(cpusTimesStatCur, cpuTimesStatCurTotal[0])

	return nil

}

func (cpuCollect *CpuCollect) GetCpuInfo() (map[string]map[string]string, error) {
	cpusInfoRet := make(map[string]map[string]string)
	curTime := time.Now().Unix()
	timediff := curTime - cpuCollect.lastTimeStamp
	cpuNum, err := cpu.Counts(false) //物理CPU的核心数

	if err != nil {
		util.GetLogger().Println(err.Error())
		return cpusInfoRet, nil
	}

	curCpusTimesStat, err := cpu.Times(true)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return cpusInfoRet, nil
	}

	cpusPercent, err := cpu.Percent(0, true)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return cpusInfoRet, nil
	}

	cpuTimesStatCurTotal, err := cpu.Times(false)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return cpusInfoRet, nil
	}

	cpuPercentTotal, err := cpu.Percent(0, false)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return cpusInfoRet, nil
	}

	curCpusTimesStat = append(curCpusTimesStat, cpuTimesStatCurTotal[0])
	cpusPercent = append(cpusPercent, cpuPercentTotal[0])

	for idx := 0; idx < len(curCpusTimesStat) && idx < len(cpusPercent); idx++ {
		cpuInfo := make(map[string]string)

		curCpuTimesStat := curCpusTimesStat[idx]
		cpuPercent := cpusPercent[idx]
		lastCpuTimesStat := cpuCollect.lastCpuTimesStat[idx]
		logicalTimediff := float64(timediff)

		if strings.Contains(curCpuTimesStat.CPU, "cpu-total") {
			logicalTimediff = float64(timediff * int64(cpuNum)) //逻辑上的时间可能要乘以cpu核数
		}

		cpuInfo["cpu"] = curCpuTimesStat.CPU
		cpuInfo["cpuUser"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.User-lastCpuTimesStat.User, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuIdle"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Idle-lastCpuTimesStat.Idle, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuSystem"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.System-lastCpuTimesStat.System, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuIrq"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Irq-lastCpuTimesStat.Irq, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuSoftirq"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Softirq-lastCpuTimesStat.Softirq, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuGuest"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Guest-lastCpuTimesStat.Guest, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuGuestNice"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.GuestNice-lastCpuTimesStat.GuestNice, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuIowait"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Iowait-lastCpuTimesStat.Iowait, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuNice"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Nice-lastCpuTimesStat.Nice, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuSteal"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Steal-lastCpuTimesStat.Steal, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuStolen"] = strconv.FormatFloat(DoValidFloatDivide(curCpuTimesStat.Stolen-lastCpuTimesStat.Stolen, logicalTimediff), 'f', -1, 64)
		cpuInfo["cpuPercent"] = strconv.FormatFloat(cpuPercent, 'f', -1, 64)

		cpusInfoRet[curCpuTimesStat.CPU] = cpuInfo
	}

	loadavg, err := GetLoadAvg()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return cpusInfoRet, nil
	}

	if err != nil {
		cpusInfoRet["cpu-total"]["cpuLoadAverage1"] = strconv.FormatFloat(-1.0, 'f', -1, 64)
		cpusInfoRet["cpu-total"]["cpuLoadAverage5"] = strconv.FormatFloat(-1.0, 'f', -1, 64)
		cpusInfoRet["cpu-total"]["cpuLoadAverage15"] = strconv.FormatFloat(-1.0, 'f', -1, 64)
		cpusInfoRet["cpu-total"]["cpuRunningProcesses"] = strconv.FormatInt(-1, 10)
		cpusInfoRet["cpu-total"]["cpuTotalProcesses"] = strconv.FormatInt(-1, 10)
		cpusInfoRet["cpu-total"]["cpuLastProcessId"] = strconv.FormatInt(-1, 10)
		util.GetLogger().Println(err.Error())
	} else {
		cpusInfoRet["cpu-total"]["cpuLoadAverage1"] = strconv.FormatFloat(loadavg.LoadAverage1*100.0, 'f', -1, 64)
		cpusInfoRet["cpu-total"]["cpuLoadAverage5"] = strconv.FormatFloat(loadavg.LoadAverage5*100.0, 'f', -1, 64)
		cpusInfoRet["cpu-total"]["cpuLoadAverage15"] = strconv.FormatFloat(loadavg.LoadAverage15*100.0, 'f', -1, 64)
		cpusInfoRet["cpu-total"]["cpuRunningProcesses"] = strconv.FormatInt(loadavg.RunningProcesses, 10)
		cpusInfoRet["cpu-total"]["cpuTotalProcesses"] = strconv.FormatInt(loadavg.TotalProcesses, 10)
		cpusInfoRet["cpu-total"]["cpuLastProcessId"] = strconv.FormatInt(loadavg.LastProcessId, 10)
	}
	cpuCollect.lastCpuTimesStat = curCpusTimesStat
	cpuCollect.lastTimeStamp = curTime

	return cpusInfoRet, nil
}

type TcpCollect struct {
	tcpCurEstab   int64
	tcpTimeAwait  int64
	tcpSnmp       map[string]int64
	lastTcpData   map[string]int64
	lastTimeStamp int64
}

func (tcpCollect *TcpCollect) TcpCollectInit() (error) {
	tcpCollect.tcpSnmp = make(map[string]int64)
	tcpCollect.lastTcpData = map[string]int64{"PassiveOpens": 0 }
	tcpCollect.lastTimeStamp = time.Now().Unix()

	return nil
}

func (tcpCollect *TcpCollect) getTcpSnmp() (error) {
	fileName := "/proc/net/snmp"
	file, err := os.Open(fileName)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	buf := bufio.NewReader(file)
	var snmpKey []string
	var snmpValue []string

	for {
		line, err := buf.ReadString('\n')

		if err == io.EOF {
			break
		}

		//Tcp: RtoAlgorithm RtoMin RtoMax MaxConn ActiveOpens PassiveOpens AttemptFails EstabResets CurrEstab InSegs OutSegs RetransSegs InErrs OutRsts InCsumErrors
		//Tcp: 1 200 120000 -1 130688 287 111 109 6 954203 954472 5289 0 1648 0
		if strings.Contains(line, "Tcp") {
			if strings.Contains(line, "CurrEstab") {
				snmpKey = strings.Split(strings.TrimSpace(line[strings.Index(line, ":")+1:]), " ")
			} else {
				snmpValue = strings.Split(strings.TrimSpace(line[strings.Index(line, ":")+1:]), " ")
			}
		}

		if len(snmpKey) > 1 && len(snmpValue) > 1 {
			break; //snmpKey snmpValue都有了,就不需要继续扫描了
		}
	}

	for i := 0; i < len(snmpKey) && i < len(snmpValue); i++ {
		tcpCollect.tcpSnmp[snmpKey[i]], _ = strconv.ParseInt(snmpValue[i], 10, 64)
	}

	return nil
}

func (tcpCollect *TcpCollect) GetTcpCurrEstab() (int64, error) {

	if err := tcpCollect.getTcpSnmp(); err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}

	tcpCollect.tcpCurEstab = tcpCollect.tcpSnmp["CurrEstab"]

	return tcpCollect.tcpCurEstab, nil
}

/*get tcp connnection increment pee second*/
func (tcpCollect *TcpCollect) GetTcpConnInc() (float64, error) {

	if err := tcpCollect.getTcpSnmp(); err != nil {
		util.GetLogger().Println(err.Error())
		return -1.0, err
	}

	curPassiveOpens := tcpCollect.tcpSnmp["PassiveOpens"]
	curTime := time.Now().Unix()
	connPsec := 0.0

	if lastTimeStamp := tcpCollect.lastTimeStamp; lastTimeStamp > 0 && curTime > lastTimeStamp {
		lastPassiveOpens := tcpCollect.lastTcpData["PassiveOpens"]
		connPsec = float64(curPassiveOpens-lastPassiveOpens) / float64(curTime-lastTimeStamp)
		tcpCollect.lastTimeStamp = curTime
		tcpCollect.lastTcpData["PassiveOpens"] = curPassiveOpens
	}

	return connPsec, nil
}

/*
sockets: used 137
TCP: inuse 6 orphan 0 tw 0 alloc 9 mem 1
UDP: inuse 2 mem 0
UDPLITE: inuse 0
RAW: inuse 0
FRAG: inuse 0 memory 0
*/
func (tcpCollect *TcpCollect) GetTimeAwait() (int64, error) {
	fileName := "/proc/net/sockstat"
	file, err := os.Open(fileName)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}

	buf := bufio.NewReader(file)
	sockstatData := make(map[string]int64)
	for {
		line, err := buf.ReadString('\n')

		if err == io.EOF {
			util.GetLogger().Println(err.Error())
			return tcpCollect.tcpTimeAwait, err
		}

		if strings.Contains(line, "TCP") {
			tmpSlice := strings.Split(strings.TrimSpace(line[strings.Index(line, ":")+1:]), " ")
			for i := 0; i < len(tmpSlice); i += 2 {
				sockstatData[tmpSlice[i]], _ = strconv.ParseInt(tmpSlice[i+1], 10, 64)
			}
			break;
		}
	}
	tcpCollect.tcpTimeAwait = sockstatData["tw"]

	return tcpCollect.tcpTimeAwait, err
}

type UdpCollect struct {
	udpSnmp       map[string]int64
	lastUdpData   map[string]int64
	lastTimeStamp int64
}

func (udpCollect *UdpCollect) UdpCollectInit() (error) {
	udpCollect.udpSnmp = make(map[string]int64)
	udpCollect.lastUdpData = map[string]int64{"inDatagrams": 0, "outDatagrams": 0, }
	udpCollect.lastTimeStamp = time.Now().Unix()

	return nil
}

func (udpCollect *UdpCollect) getUdpInfo() error {
	fileName := "/proc/net/snmp"
	file, err := os.Open(fileName)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	buf := bufio.NewReader(file)
	var snmpKey []string
	var snmpValue []string

	for {
		line, err := buf.ReadString('\n')
		if err == io.EOF {
			util.GetLogger().Println(err.Error())
			return nil
		}
		//Udp: InDatagrams NoPorts InErrors OutDatagrams RcvbufErrors SndbufErrors InCsumErrors
		//Udp: 309123 1 0 309331 0 0 0
		if strings.Contains(line, "Udp") {
			if strings.Contains(line, "InDatagrams") {
				snmpKey = strings.Split(strings.TrimSpace(line[strings.Index(line, ":")+1:]), " ")
			} else {
				snmpValue = strings.Split(strings.TrimSpace(line[strings.Index(line, ":")+1:]), " ")
			}
		}
		if len(snmpKey) > 1 && len(snmpValue) > 1 {
			break; //snmpKey snmpValue都有了,就不需要继续扫描了
		}
	}

	for i := 0; i < len(snmpKey) && i < len(snmpValue); i++ {
		udpCollect.udpSnmp[snmpKey[i]], _ = strconv.ParseInt(snmpValue[i], 10, 64)
	}

	return nil
}

func (udpCollect *UdpCollect) GetUdpInfo() (map[string]string, error) {
	udpInfoRet := make(map[string]string)
	curTime := time.Now().Unix()

	if err := udpCollect.getUdpInfo(); err != nil {
		util.GetLogger().Println(err.Error())
		return udpInfoRet, err
	}

	curUdpSnmp := udpCollect.udpSnmp
	lastUdpSnmp := udpCollect.lastUdpData
	timeDiff := float64(curTime - udpCollect.lastTimeStamp)
	//Udp: InDatagrams NoPorts InErrors OutDatagrams RcvbufErrors SndbufErrors InCsumErrors
	//Udp: 309123 1 0 309331 0 0 0
	udpInfoRet["udpInDatagrams"] = strconv.FormatFloat(DoValidFloatDivide(float64(curUdpSnmp["InDatagrams"]-lastUdpSnmp["InDatagrams"]), timeDiff), 'f', -1, 64)
	udpInfoRet["udpOutDatagrams"] = strconv.FormatFloat(DoValidFloatDivide(float64(curUdpSnmp["OutDatagrams"]-lastUdpSnmp["OutDatagrams"]), timeDiff), 'f', -1, 64)
	udpInfoRet["udpInErrors"] = strconv.FormatFloat(DoValidFloatDivide(float64(curUdpSnmp["InErrors"]-lastUdpSnmp["InErrors"]), timeDiff), 'f', -1, 64)
	udpInfoRet["udpRcvbufErrors"] = strconv.FormatFloat(DoValidFloatDivide(float64(curUdpSnmp["RcvbufErrors"]-lastUdpSnmp["RcvbufErrors"]), timeDiff), 'f', -1, 64)
	udpInfoRet["udpSndbufErrors"] = strconv.FormatFloat(DoValidFloatDivide(float64(curUdpSnmp["SndbufErrors"]-lastUdpSnmp["SndbufErrors"]), timeDiff), 'f', -1, 64)
	udpInfoRet["udpInCsumErrors"] = strconv.FormatFloat(DoValidFloatDivide(float64(curUdpSnmp["InCsumErrors"]-lastUdpSnmp["InCsumErrors"]), timeDiff), 'f', -1, 64)
	udpCollect.lastTimeStamp = curTime
	udpCollect.lastUdpData = udpCollect.udpSnmp

	return udpInfoRet, nil
}

type NetCollect struct {
	lastNetIOCountersStat []psutilnet.IOCountersStat
	lastTimeStamp         int64
}

func (netCollect *NetCollect) NetCollectInit() (error) {
	netCollect.lastTimeStamp = time.Now().Unix()
	netIOCountersStat, err := psutilnet.IOCounters(true)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	netIOCountersStatTotal, err := psutilnet.IOCounters(false)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	netCollect.lastNetIOCountersStat = append(netIOCountersStat, netIOCountersStatTotal[0])

	return nil
}

func (netCollect *NetCollect) GetNetIOCounters() (map[string]map[string]string, error) {
	curTime := time.Now().Unix()
	netInfoRet := make(map[string]map[string]string)
	netIOCountersStat, err := psutilnet.IOCounters(true)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return netInfoRet, err
	}

	netIOCountersStatTotal, err := psutilnet.IOCounters(false)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return netInfoRet, err
	}

	netIOCountersStatAll := append(netIOCountersStat, netIOCountersStatTotal[0])
	timeDiff := float64(curTime - netCollect.lastTimeStamp)

	for idx, curNetIOCountersStat := range netIOCountersStatAll {
		netInfo := make(map[string]string)
		lastNetIOCountersStat := netCollect.lastNetIOCountersStat[idx]
		netInfo["netInterfaceName"] = curNetIOCountersStat.Name
		netInfo["netSentTraffic"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.BytesSent-lastNetIOCountersStat.BytesSent), timeDiff), 'f', -1, 64)
		netInfo["netRecvTraffic"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.BytesRecv-lastNetIOCountersStat.BytesRecv), timeDiff), 'f', -1, 64)
		netInfo["netPacketsSent"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.PacketsSent-lastNetIOCountersStat.PacketsSent), timeDiff), 'f', -1, 64)
		netInfo["netPacketsRecv"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.PacketsRecv-lastNetIOCountersStat.PacketsRecv), timeDiff), 'f', -1, 64)
		netInfo["netErrin"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.Errin-lastNetIOCountersStat.Errin), timeDiff), 'f', -1, 64)
		netInfo["netErrout"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.Errout-lastNetIOCountersStat.Errout), timeDiff), 'f', -1, 64)
		netInfo["netDropin"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.Dropin-lastNetIOCountersStat.Dropin), timeDiff), 'f', -1, 64)
		netInfo["netDropout"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.Dropout-lastNetIOCountersStat.Dropout), timeDiff), 'f', -1, 64)
		netInfo["netFifoin"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.Fifoin-lastNetIOCountersStat.Fifoin), timeDiff), 'f', -1, 64)
		netInfo["netFifoout"] = strconv.FormatFloat(DoValidFloatDivide(float64(curNetIOCountersStat.Fifoout-lastNetIOCountersStat.Fifoout), timeDiff), 'f', -1, 64)
		netInfoRet[curNetIOCountersStat.Name] = netInfo
	}

	netCollect.lastTimeStamp = curTime
	netCollect.lastNetIOCountersStat = netIOCountersStatAll

	return netInfoRet, nil
}

type ProcessCollect struct {
}

func (processCollect *ProcessCollect) ProcessCollectInit() (error) {
	return nil
}

func (processCollect *ProcessCollect) GetProcessNum() (int, error) {
	proc, err := process.Pids()
	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}
	return len(proc), nil
}

/*
    D    不可中断     Uninterruptible sleep (usually IO)
    R    正在运行，或在队列中的进程
    S    处于休眠状态
    T    停止或被追踪
    Z    僵尸进程
    W    进入内存交换（从内核2.6开始无效）
    X    死掉的进程

    <    高优先级
    N    低优先级
    L    有些页被锁进内存
    s    包含子进程
    +    位于后台的进程组；
    l    多线程，克隆线程  multi-threaded (using CLONE_THREAD, like NPTL pthreads do)
*/
//返回处于状态X的进程数量,可以是多个状态的组合但是要以slice形式给出,如Ss,调用方式GetXStatsProcessNum("S","s")
func (processCollect *ProcessCollect) GetXStatsProcessNum(xStats []string) (int, error) {
	proc, err := process.Pids()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}

	xStatsNum := 0
	for pid := range proc {
		if ok, err := process.PidExists(int32(pid)); ok && err == nil {
			if p, err := process.NewProcess(int32(pid)); err == nil {
				if status, err := p.Status(); err == nil {
					tag := true
					for _, stat := range xStats {
						if strings.Contains(status, stat) == false {
							tag = false
							break
						}
					}
					if tag {
						xStatsNum++
					}
				} else {
					util.GetLogger().Println(err.Error())
					return -1, err
				}
			} else {
				util.GetLogger().Println(err.Error())
				return -1, err
			}
		} else if ok == false {
			err := errors.New("GetXStatsProcessNum:process which pid = " + string(pid) + " not exists.")
			util.GetLogger().Println(err.Error())
			return -1, err
		} else {
			util.GetLogger().Println(err.Error())
			return -1, err
		}
	}
	return xStatsNum, nil
}

/*返回processName中给定的进程的简要信息*/
func (processCollect *ProcessCollect) GetProcessInfo(processName string) (map[string]string, error) {
	processInfoRet := make(map[string]string)
	processId, err := process.Pids()

	if err != nil {
		util.GetLogger().Println(err.Error())
		return processInfoRet, err
	}

	for _, pid := range processId {
		if p, err := process.NewProcess(int32(pid)); p != nil && err == nil {
			if name, err := p.Name(); err == nil {
				if strings.Compare(name, processName) == 0 {

					if status, err := p.Status(); err == nil {
						processInfoRet["status"] = status
					} else {
						util.GetLogger().Println(err.Error() + "GetProcessInfo:can't get 'status'")
						return processInfoRet, err
					}

					if createTime, err := p.CreateTime(); err == nil {
						processInfoRet["createTime"] = strconv.FormatInt(createTime/int64(time.Second), 10)
					} else {
						util.GetLogger().Println(err.Error() + "GetProcessInfo:can't get 'createTime'")
						return processInfoRet, err
					}

					/*暂时不支持采集  在linux上尝试
					if isRunning, err := p.IsRunning(); err == nil {
						processInfoRet["isRunning"] = strconv.FormatBool(isRunning)
					} else {
						util.GetLogger().Println(err.Error() + "GetProcessInfo:can't get 'isRunning'")
						return processInfoRet, err
					}*/

					if percent, err := p.Percent(0); err == nil {
						processInfoRet["percent"] = strconv.FormatFloat(percent, 'f', -1, 64)
					} else {
						util.GetLogger().Println(err.Error() + "GetProcessInfo:can't get 'percent'")
						return processInfoRet, err
					}

					if memoryInfo, err := p.MemoryInfo(); err == nil {
						processInfoRet["Swap"] = strconv.FormatUint(memoryInfo.Swap, 10)
						processInfoRet["VMS"] = strconv.FormatUint(memoryInfo.VMS, 10)
						processInfoRet["RSS"] = strconv.FormatUint(memoryInfo.RSS, 10)
					} else {
						util.GetLogger().Println(err.Error() + "GetProcessInfo:can't get 'memoryInfo'")
						return processInfoRet, err
					}

					return processInfoRet, nil
				} //strings.Compare(name, processName) == 0
			} //name, err := p.Name(); err == nil
		} else {
			util.GetLogger().Println(err.Error() + "GetProcessInfo:NewProcess failed'")
			//return processInfoRet, err
		}
	} //for
	err = errors.New("GetProcessInfo:process " + processName + " doesn't exists.")
	util.GetLogger().Println(err.Error())
	return processInfoRet, err
}

///proc/uptime
type SystemCollect struct {
	lastRunTime float64
	curRunTime  float64
	uptimeFile  string
	hasRebooted bool //插件是否重启过
}

func (systemCollect *SystemCollect) SystemCollectInit() (error) {
	systemCollect.lastRunTime = -1.0
	//记录插件启动时间文件
	systemCollect.uptimeFile = conf.GetConfigTool().UPTIMEFile
	file, err := os.Open(systemCollect.uptimeFile)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	buf := bufio.NewReader(file)
	line, err := buf.ReadString('\n')
	//先从.UPTIME文件读取,如失败再从调用getSystemRuntime从/proc/uptime文件读取
	if err != io.EOF && err != nil {
		util.GetLogger().Println(err.Error())
		systemCollect.lastRunTime, err = systemCollect.getOSRuntime()
		if err != nil {
			util.GetLogger().Println(err.Error())
			util.GetLogger().Println(err.Error())
			return err
		}
	} else {
		if len(line) > 0 {
			systemCollect.lastRunTime, err = strconv.ParseFloat(strings.Split(line, " ")[0], 64)
			if err != nil {
				util.GetLogger().Println(err.Error())
				return err
			}
		} else {
			systemCollect.lastRunTime = 0 //文件中没有内容
		}
	}
	systemCollect.curRunTime = -1.0
	systemCollect.hasRebooted = false
	return nil
}

func (systemCollect *SystemCollect) getOSRuntime() (float64, error) {
	file, err := os.Open("/proc/uptime")

	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}

	buf := bufio.NewReader(file)
	line, err := buf.ReadString('\n')
	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}
	sysRunTime, err := strconv.ParseFloat(strings.Split(line, " ")[0], 64)
	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1, err
	}
	return sysRunTime, nil
}

func (systemCollect *SystemCollect) writeRuntimeToFile() (error) {
	curRunTimeStr := strconv.FormatFloat(systemCollect.curRunTime, 'f', -1, 64)
	err := ioutil.WriteFile(systemCollect.uptimeFile, []byte(curRunTimeStr), 0666)
	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}
	return nil
}

func (systemCollect *SystemCollect) GetRunTime() (float64, bool, error) {
	curRunTime, err := systemCollect.getOSRuntime()
	if err != nil {
		util.GetLogger().Println(err.Error())
		return -1.0, false, err
	}
	systemCollect.curRunTime = curRunTime
	if curRunTime < systemCollect.lastRunTime {
		systemCollect.hasRebooted = true
		systemCollect.lastRunTime = curRunTime
	} else {
		systemCollect.hasRebooted = false
	}
	err = systemCollect.writeRuntimeToFile()
	if err != nil {
		util.GetLogger().Println(err.Error())
		return systemCollect.curRunTime, systemCollect.hasRebooted, err
	}
	return systemCollect.curRunTime, systemCollect.hasRebooted, nil
}
