<template>
  <div class="container">
    <div class="row">
      <h1>Modell evaluieren</h1>
      <div class="col-md">
      </div>
      <div class="col-md">
        <div class="rContainer">
          <h2>Accuracy = {{accuracy}}%</h2>
          <h2>Loss = {{loss}}%</h2>
          <span>Wenn Sie mit der Modell Performance einverstanden sind klicken Sie bitte auf weiter, falls nicht gehen Sie zurück und passen den Datensatz und die Anzahl der Epochen an und Trainieren das Modell erneut.</span>

          <div>
            <router-link to="/page4">
              <button type="Button" class="btn btn-danger">Zurück</button>
            </router-link>
            <router-link to="/page6">
              <button type="Button" class="btn btn-primary">Weiter</button>
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
    let currentObj = this;
    this.axios.get("http://127.0.0.1:5000/check?key=eval")
        .then(function (response) {
          currentObj.accuracy = response.data[0]
          currentObj.loss = response.data[1]
        })
        .catch(function (error) {
        });
  },
  methods: {},
  name: "page5",
  data: function () {
    return {
      accuracy: 0,
      loss: 0
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
