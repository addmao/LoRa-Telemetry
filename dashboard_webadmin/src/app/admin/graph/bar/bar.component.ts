import { Component, OnInit, Input, OnChanges, SimpleChanges } from '@angular/core';
import { ApiResponse, Flow, Graph, GraphData } from '../../../model/apiResponse';

@Component({
  selector: 'app-bar-env',
  templateUrl: './bar.component.html',
  styleUrls: ['./bar.component.css']
})
export class BarComponentEnv implements OnInit, OnChanges {

  @Input() data: any;
  @Input() name: string;
  chartOption:any;
  constructor() { }

  ngOnInit() {
  }

  ngOnChanges(changes: SimpleChanges) {
    this.chartOption = {
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'cross',
                crossStyle: {
                    color: '#999'
                }
            }
        },
        toolbox: {
            feature: {
                dataView: {show: true, readOnly: false},
                magicType: {show: true, type: ['line', 'bar']},
                restore: {show: true},
                saveAsImage: {show: true}
            }
        },
        legend: {
            data:['Humid','Temp','RSSI']
        },
        xAxis: [
            {
                type: 'category',
                data: this.data['xaxis'],
                axisPointer: {
                    type: 'shadow'
                }
            }
        ],
        yAxis: [
            {
                type: 'value',
                name: 'Temperature(C)/Humidity(%)',
                axisLabel: {
                    formatter: '{value}'
                }
            },
            {
                type: 'value',
                name: 'RSSI (dBm)',

                interval: 5,
                axisLabel: {
                    formatter: '{value}'
                }
            }
        ],
        series: [
            {
                name:'Humid',
                type:'bar',
                data: this.data['humid']
            },
            {
                name:'Temp',
                type:'bar',
                data:this.data['temp']
            },
            {
                name:'RSSI',
                type:'line',
                yAxisIndex: 1,
                data:this.data['rssi']
            }
        ]
    };
  }

}

@Component({
  selector: 'app-bar-water',
  templateUrl: './bar.component.html',
  styleUrls: ['./bar.component.css']
})
export class BarComponentWater implements OnInit, OnChanges {

  @Input() data: any;
  @Input() name: string;
  chartOption:any;
  constructor() { }

  ngOnInit() {
  }

  ngOnChanges(changes: SimpleChanges) {
    console.log(changes);
    this.chartOption = {
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'cross',
                crossStyle: {
                    color: '#999'
                }
            }
        },
        toolbox: {
            feature: {
                dataView: {show: true, readOnly: false},
                magicType: {show: true, type: ['line', 'bar']},
                restore: {show: true},
                saveAsImage: {show: true}
            }
        },
        legend: {
            data:['Water Level']
        },
        xAxis: [
            {
                type: 'category',
                data: this.data['xaxis'],
                axisPointer: {
                    type: 'shadow'
                }
            }
        ],
        yAxis: [
            {
                type: 'value',
                name: 'Level',
                axisLabel: {
                    formatter: '{value}'
                }
            }
        ],
        series: [
            {
                name:'Water Level',
                type:'line',
                data: this.data['level']
            }
        ]
    };
  }

}
