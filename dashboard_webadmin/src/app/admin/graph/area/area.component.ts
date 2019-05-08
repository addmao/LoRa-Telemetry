import { Component, OnInit, Input, OnChanges, SimpleChanges } from '@angular/core';

@Component({
  selector: 'app-area',
  templateUrl: './area.component.html',
  styleUrls: ['./area.component.css']
})
export class AreaComponent implements OnInit, OnChanges {

  @Input() data: any;
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
          label: {
            backgroundColor: '#6a7985'
          }
        }
      },
  
      grid: {
        left: '3%',
        right: '4%',
        bottom: '3%',
        containLabel: true
      },
      xAxis: [
        {
          type: 'category',
          boundaryGap: false,
          data: this.data.chart.label
        }
      ],
      yAxis: [
        {
          type: 'value'
        }
      ],
      series: [
        {
          name: 'Marketing',
          type: 'line',
          stack: 'overview',
          areaStyle: {},
          data: this.data.chart.value
        },
        {
          name: 'IT',
          type: 'line',
          stack: 'overview',
          areaStyle: {},
          data: [220, 182, 191, 234, 290, 330, 310, 330, 390, 340, 330, 400]
        },
        {
          name: 'Hr',
          type: 'line',
          stack: 'overview',
          areaStyle: {},
          data: [150, 232, 201, 154, 190, 330, 410, 410, 440, 470, 500, 300]
        },
        {
          name: 'Sale',
          type: 'line',
          stack: 'overview',
          areaStyle: {},
          data: [320, 332, 301, 334, 390, 330, 320, 330, 330, 340, 200, 230]
        }
      ]
    };
  }

}
