/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;
open Reason_jsonrpc;
open Rench;
/* open Revery; */

module Protocol = ExtensionHostProtocol;

type t = {
  process: NodeProcess.t,
  rpc: Rpc.t,
};

let emptyJsonValue = `Assoc([]);

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

type messageHandler =
  (string, string, Yojson.Safe.json) =>
  result(option(Yojson.Safe.json), string);
let defaultMessageHandler = (_, _, _) => Ok(None);

let start =
    (
      ~initData=ExtensionHostInitData.create(),
      ~onInitialized=defaultCallback,
      ~onMessage=defaultMessageHandler,
      ~onClosed=defaultCallback,
      setup: Setup.t,
    ) => {
  let args = ["--type=extensionHost"];
  let env = [
    "AMD_ENTRYPOINT=vs/workbench/services/extensions/node/extensionHostProcess",
    "VSCODE_PARENT_PID=" ++ string_of_int(Process.pid()),
  ];
  let process =
    NodeProcess.start(~args, ~env, setup, setup.extensionHostPath);

  let lastReqId = ref(0);
  let rpcRef = ref(None);

  let send =
      (
        ~msgType=ExtensionHostProtocol.MessageType.requestJsonArgs,
        msg: Yojson.Safe.json,
      ) => {
    switch (rpcRef^) {
    | None => prerr_endline("RPC not initialized.")
    | Some(v) =>
      incr(lastReqId);
      let reqId = lastReqId^;

      let request =
        `Assoc([
          ("type", `Int(msgType)),
          ("reqId", `Int(reqId)),
          ("payload", msg),
        ]);

      Rpc.sendNotification(v, "ext/msg", request);
    };
  };

  let handleMessage = (_reqId: int, payload: Yojson.Safe.json) =>
    switch (payload) {
    | `List([`String(scopeName), `String(methodName), args]) =>
      let _ = onMessage(scopeName, methodName, args);
      ();
    | _ =>
      print_endline("Unknown message: " ++ Yojson.Safe.to_string(payload))
    /* switch (onMessage(id, payload)) { */
    /* | Ok(None) => () */
    /* | Ok(Some(_)) => */
    /*   /1* TODO: Send response *1/ */
    /*   () */
    /* | Error(_) => */
    /*   /1* TODO: Send error *1/ */
    /*   () */
    /* }; */
    };

  let _sendInitData = () => {
    send(
      ~msgType=Protocol.MessageType.initData,
      ExtensionHostInitData.to_yojson(initData),
    );
  };

  let _handleInitialization = () => {
    onInitialized();
    /* Send workspace and configuration info to get the extensions started */
    open ExtensionHostProtocol.OutgoingNotifications;

    Configuration.initializeConfiguration() |> send;
    Workspace.initializeWorkspace("onivim-workspace-id", "onivim-workspace")
    |> send;
  };

  let onNotification = (n: Notification.t, _) => {
    switch (n.method, n.params) {
    | ("host/msg", json) =>
      open Protocol.Notification;
      print_endline("JSON: " ++ Yojson.Safe.to_string(json));
      switch (parse(json)) {
      | Request(req) => handleMessage(req.reqId, req.payload)
      | Reply(_) => ()
      | Ack(_) => ()
      | Ready => _sendInitData()
      | Initialized => _handleInitialization()
      };

    | _ =>
      print_endline("[Extension Host Client] Unknown message: " ++ n.method)
    };
  };

  let onRequest = (_, _) => Ok(emptyJsonValue);

  let rpc =
    Rpc.start(
      ~onNotification,
      ~onRequest,
      ~onClose=onClosed,
      process.stdout,
      process.stdin,
    );

  rpcRef := Some(rpc);

  {process, rpc};
};

let pump = (v: t) => Rpc.pump(v.rpc);