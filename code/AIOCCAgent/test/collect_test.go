package test

import (
	"testing"
	"fmt"
 	//"strings"
 )

/*
func TestDisk(t *testing.T) {
	resp, err := http.Get("169.254.169.254/2009-04-04/meta-data/instance-id")
	if err != nil {
		// handle error
	}

	body, err := ioutil.ReadAll(resp.Body)
	fmt.Println(body)

}

func TestCpu(t *testing.T) {
	cpuTimesStat, _ := cpu.Times(false)
	fmt.Printf("%v\n",cpuTimesStat)
	cpuPercent,_:=cpu.Percent(0,false)
	fmt.Printf("%v\n",cpuPercent)
}

func GetXStatsProcessNum(xStats []string) (int, error) {
	proc, err := process.Pids()

	if err != nil {
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
				}
			}
		}
	}

	return xStatsNum, nil
}

func TestCpu(t *testing.T) {
	num, _ := GetXStatsProcessNum([]string{"S", "N", "s"})
	fmt.Println(num)
}


func TestCpu(t *testing.T) {
	proc, _ := process.Pids()
	for pid := range proc {
		if ok, _ := process.PidExists(int32(pid)); ok {
			p, _ := process.NewProcess(int32(pid))
			name, _ := p.Name()
		//	if strings.Contains(name,"aiocc")  {
				status, _ := p.Status()
				fmt.Printf("%v %v\n", name, status)
			//}
		}else {
			//fmt.Println("不存在")
		}
	}


}

*/

func TestCpu(t *testing.T) {
	bt:= make(map[string]float64)
	names:=[]string{"a","b","c"}
	for _,k := range names{
		//if k{
			bt[k]+=9.0
		//}
	}
	for _,k := range names{
		fmt.Println(bt[k])
	}
	fmt.Println(bt["sss"])
}
