CTR_TYPE_OTOBJECT = 0;
CTR_TYPE_OTNIL = 1;
CTR_TYPE_OTSTRING = 2;
CTR_TYPE_OTBOOL = 0;
CTR_TYPE_OT = 0;
var ctrObject = {
  "info": {"sticky": null, "type": CTR_TYPE_OTOBJECT}
  "new": () => {return new ctrObject();},
  "_init": () => {this.context = new Context(); this.context._init();},
  "isNil": () => {return False;},
  "toBoolean": () => {return True;},
  "toNumber": () => {return 1;},
  "toString": () => {return "[Object]";},
  "fromComputer:": (ip) => {},
  "learn:means:": (sel, rep) => {},
  "message:arguments:": (sel, args) => {},
  "do": () => {},
  "done": () => {},
  "myself": () => {return this;},
  "type": () => "Object",
  "respondTo:": (sel) => {},
  "respondTo:and:": (sel, msg) => {},
  "respondTo:and:and:": (sel, msg0, msg1) => {},
  "on:do:": (sel, blk) => {},
  "equals:": (other) => {}
  "=": (other) => {}
};
