<template>
  <div class="container">
    <div class="row">
      <h1>Sichtbereich anpassen.</h1>
      <div class="col-md">
      </div>
      <div class="col-md">

        <div class="rContainer">
          <form>
            <div class="mb-3">
              <label for="device_ip" class="form-label">Device IP</label>
              <input v-model="device_ip" type="text" class="form-control" id="device_ip">
            </div>
          </form>


          <div class="stream">
            <img :src="'http://'+device_ip+'/capture?rand='+rand" :key="rand">
          </div>
          <div>
            <button type="Button" @click="refresh()" class="btn btn-primary">Refresh Image</button>
          </div>
          <div>
            <router-link to="/page7">
              <button type="Button" class="btn btn-danger">Zurück</button>
            </router-link>
            <router-link to="/page9">
              <button type="Button" @click="disable_cam()" class="btn btn-primary">Weiter</button>
            </router-link>

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
  },
  methods: {
    disable_cam() {
      let currentObj = this;
      this.axios.get("http://127.0.0.1:5000/do?key=stop&ip="+this.device_ip).then(function (response) {
        currentObj.output = response.data;
        currentObj.ip = response.data
      }).catch(function (error) {
        currentObj.output = error;
      });
      if (!this.cam_disabled) {
        this.cam_disabled = true;
        console.log("Cam disabled")
      }

    },
    refresh() {
      this.rand++;
    }
  },
  name: "page8",
  data: function () {
    return {
      device_ip: null,
      rand: 1,
      cam_disabled: false
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
