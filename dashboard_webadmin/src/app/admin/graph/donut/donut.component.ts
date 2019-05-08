import { Component, OnInit, Input, OnChanges, SimpleChanges } from '@angular/core';

@Component({
  selector: 'app-donut',
  templateUrl: './donut.component.html',
  styleUrls: ['./donut.component.css']
})
export class DonutComponent implements OnInit, OnChanges {

  @Input() data: any;
  @Input() name: string;
  chartOption:any;
  constructor() { }

  ngOnInit() {
  }

  ngOnChanges(changes: SimpleChanges) {
    console.log(changes);
    this.chartOption =  {
      tooltip: {
          trigger: 'item',
          formatter: "{a} <br/>{b}: {c} ({d}%)"
      },
      legend: {
        orient: 'vertical',
        x: 'right',
        type: 'scroll',
    },
      series: [
          {
              name:this.name,
              type:'pie',
              radius: ['50%', '70%'],
              avoidLabelOverlap: true,
              label: {
                show: true,
                formatter: '{b} ({c}%)',
              },
              labelLine: {
                show: true,
                length: 40,
                length2: 20,
                smooth: false,
              },
              data:this.data
          }
      ]
    };
  }


}
