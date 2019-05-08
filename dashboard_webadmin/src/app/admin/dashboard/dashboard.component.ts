import { Component, OnInit } from '@angular/core';
import { ApiResponse, Flow, Graph, GraphData, DonutData } from '../../model/apiResponse';
import { DashboardService } from '../../service/dashboard.service';

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
    xaxis: [],
  };
  isLoading = true;
  constructor(
    private dashboardService: DashboardService
  ) { }

  ngOnInit() {
    this.dashboardService.get().subscribe(res => {

      for(let i in res){
        this.data.rssi.push(res[i]['rssi']);
        this.data.temp.push(res[i]['temp']);
        this.data.humid.push(res[i]['humid']);
        this.data.xaxis.push(res[i]['datetime']);
      }
      console.log(this.data)
      this.isLoading = false;
    })
  }

}
