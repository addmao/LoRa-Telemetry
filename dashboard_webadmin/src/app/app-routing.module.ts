import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { DashboardComponent } from './admin/dashboard/dashboard.component';


const routes: Routes = [
  {
    path: '',
    data: {
        title: 'Dashboard'
    },
    children: [
      {
        path: '',
        component: DashboardComponent
      }
    ]
  }

];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
