import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TwoDonutComponent } from './two-donut.component';

describe('TwoDonutComponent', () => {
  let component: TwoDonutComponent;
  let fixture: ComponentFixture<TwoDonutComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ TwoDonutComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TwoDonutComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
