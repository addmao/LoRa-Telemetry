export class ApiResponse{
    topDownloadUser:Graph[];
    topUploadUser:Graph[];
    topExternalServer:Graph[];
    topInternalServer:Graph[];
    topExternalService:Graph[];
    topInternalService:Graph[];
    topInternalServerName:Graph[];
    TopBrowserAgent:Graph[];
    TopWebUasge:Graph[];
    TopWebHost:Graph[];
    TopWebServer:Graph[];
    IPVertionRatio:Graph[];
    TopWebQueryMethod:Graph[];
    TopUserUsage:Graph[];
    TopServerOwner:Graph[];
    TopAgentDetaill:Graph[];
    TopBrowser:Graph[];
    TopClientOS:Graph[];
}

export class Result{
    result:Flow;
}

export class Flow{
    topDownloadUser:Graph[];
    topUploadUser:Graph[];
    topExternalServer:Graph[];
    topInternalServer:Graph[];
    topExternalService:Graph[];
    topInternalService:Graph[];
    topInternalServerName:Graph[];
    TopBrowserAgent:Graph[];
    TopWebUasge:Graph[];
    TopWebHost:Graph[];
    TopWebServer:Graph[];
    IPVertionRatio:Graph[];
    TopWebQueryMethod:Graph[];
    TopUserUsage:Graph[];
    TopServerOwner:Graph[];
    TopAgentDetaill:Graph[];
    TopBrowser:Graph[];
    TopClientOS:Graph[];
}

export class Graph{
    value:number;
    key:string;
}

export class GraphData{
    value:number[];
    label:string[];
}

export class DonutData{
    value:number;
    name:string;
}
