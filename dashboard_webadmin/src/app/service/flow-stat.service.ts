import { Injectable } from '@angular/core';
import { HttpClient, HttpParams } from '@angular/common/http';
import { api as apiConst } from '../constant';

@Injectable({
  providedIn: 'root'
})
export class FlowStatService {

  constructor(private http: HttpClient) { }
  test() {
    return this.http.get(apiConst.TEST_URL);
  }

  getFlow(start,end) {
    return this.http.get(apiConst.HOST + apiConst.FLOW_URL + start + '/'+end);
  }
}