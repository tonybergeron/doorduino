/* Doorduino */
/* -  offers Web Based browser UI for opening the door - */

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"

#define VERSION_STRING "1.0"

// CHANGE THIS TO YOUR OWN UNIQUE VALUE
static uint8_t mac[6] = { 0x02, 0xAA, 0xBB, 0xCC, 0x00, 0x22 };

// CHANGE THIS TO MATCH YOUR HOST NETWORK
static uint8_t ip[4] = { 192, 168, 1, 222 }; // area 51!

P(Page_start) = "<html><head><title>Web_Parms_1 Version " VERSION_STRING "</title></head><body>\n";
P(Page_end) = "</body></html>";
P(Get_head) = "<h1>GET from ";
P(Post_head) = "<h1>POST to ";
P(Unknown_head) = "<h1>UNKNOWN request for ";
P(Default_head) = "unidentified URL requested.</h1><br>\n";
P(Raw_head) = "raw.html requested.</h1><br>\n";
P(Parsed_head) = "parsed.html requested.</h1><br>\n";
P(Good_tail_begin) = "URL tail = '";
P(Bad_tail_begin) = "INCOMPLETE URL tail = '";
P(Tail_end) = "'<br>\n";
P(Parsed_tail_begin) = "URL parameters:<br>\n";
P(Parsed_item_separator) = " = '";
P(Params_end) = "End of parameters<br>\n";
P(Post_params_begin) = "Parameters sent by POST:<br>\n";
P(Line_break) = "<br>\n";

/* all URLs on this server will start with "" because of how we
 * define the PREFIX value.  We also will listen on port 80, the
 * standard HTTP service port */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

#define DOOR_PIN 7

int door = 0;             //integer for door
int doorPOST = 0;         //integer of the value from the POST
int timeout = 4000;       //integer for door close timeout
int doorOpenVal = 200;    //integer for the pin to open the door
int doorCloseVal = 0;     //integer for the pint to close the door

/* Home Page GET */
void doorCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();

  /* we don't output the body for a HEAD request */
  if (type == WebServer::GET)
  {
    /* store the HTML in program memory using the P macro */
    P(message) = 
      "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='apple-mobile-web-app-capable' content='yes'/><meta http-equiv='X-UA-Compatible' content='IE=edge,chrome=1'><meta name='viewport' content='width=device-width, user-scalable=no'><title>Doorduino</title><link rel='stylesheet' href='http://code.jquery.com/mobile/1.4.2/jquery.mobile-1.4.2.min.css'/><link href='data:image/x-icon;base64,AAABAAEAEBAQAAAAAAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAD3zosAAAAAAPD29wBwgIIAip2hAN7e3gBbWl4AV1ReAOvt0wDS1tYARkRKAPv3/wDt7e0AAAAAAAAAAAAAAAAAFVVVVVVVVVEVu7u7sAC7URW7u7uwALtRFbu7u7AAu1EVu4ibswC7URW7hJuwALtRFbuEm7AAu1Ely5mbsAC8UjJcu7u7u8UjoyXLu7u8UjoaMly7u8UjoRGjJcu8UjoREWcyW7UjoRERpqMlUjoRERFqGjIjoREREaYRozoRERGAAQAAgAEAAIABAACAAQAAgAEAAIABAACAAQAAAAAAAAAAAAAAAAAAgAEAAMADAADABwAAwA8AAMgfAADMPwAA' rel='icon' type='image/x-icon'/><script src='http://code.jquery.com/jquery-1.9.1.min.js'></script><script src='http://code.jquery.com/mobile/1.4.2/jquery.mobile-1.4.2.min.js'></script><style>body, .ui-page{height:99%;font-size: 1.2em;}.ui-body{height:90%;}#doorButton{display: block;margin: 10px;}.info{text-align:center;}.ui-btn{height: 140px;font-size: 48px;line-height:280%;}.door-closed{background-color: rgba(255,0,0,.25)}.door-open{background-color: rgba(0,255,0,.25)}</style><script>$(document).ready(function(){var timerid;var duration=4;var loading=function(){$.mobile.loading( 'show',{text: 'Connecting', textVisible: true, theme: 'b', textonly: false, html: ''});};/* Timer for the door being opened */ var timer=function(time){var sec=time;$('#status').text('Door Closes in: ' + sec--);timerid=setInterval(function(){/* $('.ui-loader h1').text('Door Opening: ' + sec--);*/ $('#status').text('Door Closes in: ' + sec--);if (sec==-1){enableDoorButton();clearInterval(timerid);}}, 1000);};/* Turns off the door button */ var disableDoorButton=function(){/* $('#status').text('Door Opening');*/ $('#doorButton').button('disable');};/* Send the state that the door is being opened */ var openDoor=function(){$('#status').addClass('door-open').removeClass('door-closed');timer(duration);};/* Turns on the door button again */ var enableDoorButton=function(){$('#doorButton').button('enable');$('#status').text('Door Closed');$('#status').addClass('door-closed').removeClass('door-open');};/* Error opening the door */ var errorDoor=function(text){setTimeout(function(){enableDoorButton();}, 2000);$('#status').text(text);};$('#doorButton').button().click(function(event){$.ajax({url: '/opendoor', type: 'GET', timout: 500, beforeSend: function(){disableDoorButton();loading();}, success: function(result){openDoor();}, error: function(XMLHttpRequest, textStatus, errorThrown){errorDoor('Error opening door. Status: ' + textStatus);}, complete: function(){$.mobile.loading('hide');}});});});</script></head><body><div data-role='page' data-theme='b'><div data-role='header' data-position='inline'><span style='font-size:1.1em;text-align:center'><h1>Doorduino</h1></span></div><div data-role='content'><div id='status' class='info ui-corner-all door-closed'>Door Closed</div><input type='button' style='height:100%' id='doorButton' value='Open Door'></input></div></div></body></html>";
      
    server.printP(message);
  }
}

/* Handles the Opening of the door */
void openDoorCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();

  /* we don't output the body for a HEAD request */
  if (type == WebServer::GET)
  {
    /* Open the Door */
    openDoor();
  }
}

void my_failCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  /* this line sends the "HTTP 400 - Bad Request" headers back to the
     browser */
  server.httpFail();

  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type == WebServer::HEAD)
    return;

  server.printP(Page_start);
  switch (type)
    {
    case WebServer::GET:
        server.printP(Get_head);
        break;
    case WebServer::POST:
        server.printP(Post_head);
        break;
    default:
        server.printP(Unknown_head);
    }

    server.printP(Default_head);
    server.printP(tail_complete ? Good_tail_begin : Bad_tail_begin);
    server.print(url_tail);
    server.printP(Tail_end);
    server.printP(Page_end);

}

void setup()
{
  pinMode(DOOR_PIN, OUTPUT);

  // Serial.begin(9600);

  // setup the Ethernet library to talk to the Wiznet board
  Ethernet.begin(mac, ip);

  /* register our default command (activated with the request of
   * http://x.x.x.x/door */
  webserver.setDefaultCommand(&doorCmd);
  webserver.addCommand("home", &doorCmd);
  webserver.addCommand("opendoor", &openDoorCmd);
  webserver.setFailureCommand(&my_failCmd);

  /* start the server to wait for connections */
  webserver.begin();
}


// Close turn off the door
void openDoor()
{
  door = doorOpenVal;
  analogWrite(DOOR_PIN, door);
  delay(timeout);
  closeDoor();
}

// Close turn off the door
void closeDoor()
{
  door = doorCloseVal;
  analogWrite(DOOR_PIN, door);
}

void loop()
{
  // process incoming connections one at a time forever
  webserver.processConnection();

  //Write the door pin value
  analogWrite(DOOR_PIN, door);
}
