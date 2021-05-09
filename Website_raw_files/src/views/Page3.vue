<template>
  <div class="container">
    <div class="row">
      <h1>Konfigurationsparameter Eingeben</h1>
      <div class="col-md">
      </div>
      <div class="col-md">
        <div class="rContainer">
          <form>
            <div class="mb-3">
              <label for="device_port" class="form-label">Device Port</label>
              <input v-model="device_port" type="text" class="form-control" id="device_port">
            </div>

            <div class="mb-3">
              <label for="wlan_ssid" class="form-label">WLAN SSID</label>
              <input v-model="wlan_ssid" type="text" class="form-control" id="wlan_ssid">
            </div>

            <div class="mb-3">
              <label for="wlan_psk" class="form-label">WLAN Schlüssel</label>
              <input v-model="wlan_psk" type="text" class="form-control" id="wlan_psk">
            </div>

            <div class="mb-3">
              <label for="mqtt_server" class="form-label">MQTT Server</label>
              <input v-model="mqtt_server" type="text" class="form-control" id="mqtt_server">
            </div>

            <div class="mb-3">
              <label for="mqtt_port" class="form-label">MQTT Port</label>
              <input v-model="mqtt_port" type="text" class="form-control" id="mqtt_port">
            </div>

            <div class="mb-3">
              <label for="mqtt_user" class="form-label">MQTT User</label>
              <input v-model="mqtt_user" type="text" class="form-control" id="mqtt_user">
            </div>

            <div class="mb-3">
              <label for="mqtt_pass" class="form-label">MQTT Passwort</label>
              <input v-model="mqtt_pass" type="text" class="form-control" id="mqtt_pass">
            </div>

            <div class="mb-3">
              <label for="mqtt_topic" class="form-label">MQTT Topic</label>
              <input v-model="mqtt_topic" type="text" class="form-control" id="mqtt_topic">
            </div>

            <div class="mb-3">
              <label for="model_epochs" class="form-label">Model Epochs</label>
              <input v-model="model_epochs" type="text" class="form-control" id="model_epochs">
            </div>

          </form>

          <router-link to="/page2a">
            <button type="Button" class="btn btn-danger">Zurück</button>
          </router-link>
          <router-link to="/page4">
            <button @click="handle_form()" type="Button" class="btn btn-primary">Weiter</button>
          </router-link>

        </div>


      </div>
      <div class="col-md">
      </div>
    </div>
  </div>
</template>
<script>
export default {
  name: "page3",
  methods: {
    handle_form() {
      console.log("Handle Form")
      let currentObj = this;
      this.axios.post("http://127.0.0.1:5000/config", {
        "wlan_ssid": this.wlan_ssid,
        "wlan_psk": this.wlan_psk,
        "device_port": this.device_port,
        "mqtt_server": this.mqtt_server,
        "mqtt_port": this.mqtt_port,
        "mqtt_user": this.mqtt_user,
        "mqtt_pass": this.mqtt_pass,
        "mqtt_topic": this.mqtt_topic,
        "model_epochs": this.model_epochs,
      }).then(function (response) {
        currentObj.output = response.data;
      }).catch(function (error) {
        currentObj.output = error;
      });
    }
  },
  data: function () {
    return {
      device_port: "/dev/ttyUSB0",
      wlan_ssid: null,
      wlan_psk: null,
      mqtt_server: "81.169.182.150",
      mqtt_port: "1883",
      mqtt_user: "raphael",
      mqtt_pass: "parkplatz",
      mqtt_topic: "classification",
      model_epochs: 20
    }
  }
}
</script>

<style>
.rContainer {
  height: 100%;
  width: 100%;
  display: inline-block;
  border-style: solid;
  border-radius: 10px;
  border-width: 2px;
  box-shadow: 5px 10px 18px #888888;
  margin: 5px;
  padding: 20px 20px;
}
</style>
