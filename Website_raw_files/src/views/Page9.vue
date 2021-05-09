<template>
  <div class="container">
    <div class="row">
      <h1>Ergebnisse</h1>
      <div class="col-md">
      </div>
      <div class="col-md">
        <div class="rContainer">
          <div v-if="mqtt_state">
            <h1>Aktuell wird Klasse {{ this.state }} vorhergesagt.</h1>
          </div>
          <div>
            <button @click="mqtt('stop')" type="Button" class="btn btn-danger">MQTT stoppen</button>
            <button @click="mqtt('start')" type="Button" class="btn btn-primary">MQTT starten</button>
          </div>


        </div>


      </div>
      <div class="col-md">
      </div>
    </div>
  </div>
</template>
<script>
export default {
  mounted: function () {
    this.timer = setInterval(() => {
      this.check_state()
    }, 2000)
  },
  methods: {
    mqtt(mqtt_switch) {

      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/mqtt?key=" + mqtt_switch).then(function (response) {
        currentObj.mqtt_state = response.data;
      }).catch(function (error) {
        currentObj.output = error;
      });


    },

    check_state() {
      console.log("check_state")
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/check?key=prediction").then(function (response) {
        currentObj.state = response.data;
      }).catch(function (error) {
        currentObj.output = error;
      });
    }
  },
  name: "page9",
  data: function () {
    return {
      state: null,
      mqtt_state: false
    }
  }
}
</script>

<style>
.stream {
  margin: 55px;
}

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
