import { Component, OnInit, Input, OnChanges, SimpleChanges } from '@angular/core';

@Component({
  selector: 'app-two-donut',
  templateUrl: './two-donut.component.html',
  styleUrls: ['./two-donut.component.css']
})
export class TwoDonutComponent implements OnInit, OnChanges {

  @Input() data: any;
  chartOption: any;
  constructor() { }

  ngOnInit() {
  }

  ngOnChanges(changes: SimpleChanges) {
    console.log(changes);
    this.chartOption = {
      tooltip: {
        trigger: 'item',
        formatter: "{a} <br/>{b}: {c} ({d}%)"
      },
      legend: {
        type: 'scroll',
        orient: 'vertical',
        x: 'right',
      },
      grid: {
        right: '80%',
        left: "10%"
      },
      series: [
        {
          name: '访问来源',
          type: 'pie',
          selectedMode: 'single',
          radius: ['29%', '49%'],
          center: ['40%', '50%'],
          label: {
            show: true,
            formatter: '{b} ({c}%)'
          },
          labelLine: {
            show: true,
            length: 50,
            length2: 20,
            smooth: false,
          },
          data: [
            { value: 335, name: '直达' },
            { value: 679, name: '营销广告' },
            { value: 1548, name: '搜索引擎' }
          ]
        },
        {
          name: '访问来源',
          type: 'pie',
          radius: ['50%', '70%'],
          center: ['40%', '50%'],
          label: {
            show: true,
            formatter: '{b} ({c}%)'
          },
          labelLine: {
            show: true,
            length: 40,
            length2: 20,
            smooth: false,
          },
          data: [
            { value: 335, name: '直达2' },
            { value: 310, name: '邮件营销' },
            { value: 234, name: '联盟广告' },
            { value: 135, name: '视频广告' },
            { value: 1048, name: '百度' },
            { value: 251, name: '谷歌' },
            { value: 147, name: '必应' },
            { value: 102, name: '其他' }
          ]
        }
      ]
    };
  }


}

