import { Injectable } from '@angular/core';
import { HttpClient, HttpParams } from '@angular/common/http';
import { api as apiConst } from '../constant';

@Injectable({
  providedIn: 'root'
})
export class DashboardService {

  constructor(private http: HttpClient) { }
  test() {
    return this.http.get(apiConst.TEST_WEBSTAT_URL);
  }

  get() {
    return this.http.get(apiConst.HOST);
  }
}