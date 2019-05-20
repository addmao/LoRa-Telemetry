import { Component, OnInit } from '@angular/core';
import { ApiResponse, Flow, Graph, GraphData, DonutData } from '../../model/apiResponse';
import { DashboardService } from '../../service/dashboard.service';
import { timer } from 'rxjs';

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.css']
})
export class DashboardComponent implements OnInit {

  data = {
    rssi: [],
    temp: [],
    humid: [],
    level: [],
    xaxis: [],
  };
  isLoading = true;
  constructor(
    private dashboardService: DashboardService
  ) { }

  ngOnInit() {
    var trigger = timer(1000,10000);
    trigger.subscribe(val => {
      this.dashboardService.get().subscribe(res => {
        this.data = {
          rssi: [],
          temp: [],
          humid: [],
          level: [],
          xaxis: [],
        };
        for(let i in res){
          this.data.rssi.unshift(res[i]['rssi']);
          this.data.temp.unshift(res[i]['temp']);
          this.data.humid.unshift(res[i]['humid']);
          this.data.level.unshift(res[i]['water_level']);
          this.data.xaxis.unshift(res[i]['datetime']);
        }
        console.log(this.data)
        this.isLoading = false;
      });
    });
  }

}
