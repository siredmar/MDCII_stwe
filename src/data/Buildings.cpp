#include <magic_enum.hpp>
#include "Buildings.h"
#include "Log.h"
#include "cod/CodParser.h"

mdcii::data::Buildings::Buildings(std::shared_ptr<cod::CodParser> t_cod)
    : m_cod{ std::move(t_cod) }
{
    Log::MDCII_LOG_DEBUG("[Buildings::Buildings()] Create Buildings.");

    GenerateBuildings();
}

void mdcii::data::Buildings::GenerateBuildings()
{
    Log::MDCII_LOG_DEBUG("[Buildings::GenerateBuildings()] Generate buildings...");

    for (auto i{ 0 }; i < m_cod->objects.object_size(); ++i)
    {
        const auto& obj{ m_cod->objects.object(i) };
        if (obj.name() == "HAUS")
        {
            for (auto j{ 0 }; j < obj.objects_size(); ++j)
            {
                const auto building{ GenerateBuilding(&obj.objects(j)) };
                buildingsMap.emplace(building.id, building);
            }
        }
    }

    Log::MDCII_LOG_DEBUG("[Buildings::GenerateBuildings()] The buildings were generated successfully.");
}

mdcii::data::Building mdcii::data::Buildings::GenerateBuilding(const cod_pb::Object* t_obj) const
{
    Building building;

    if (t_obj->has_variables())
    {
        for (auto i{ 0 }; i < t_obj->variables().variable_size(); ++i)
        {
            const auto var{ t_obj->variables().variable(i) };
            if (var.name() == "Id")
            {
                if (var.value_int() == 0)
                {
                    building.id = 0;
                }
                else
                {
                    building.id = var.value_int() - 20000;
                }
            }
            else if (var.name() == "Gfx")
            {
                building.gfx = var.value_int();
            }
            else if (var.name() == "Blocknr")
            {
                building.blocknr = var.value_int();
            }
            else if (var.name() == "Kind")
            {
                const auto kind{ magic_enum::enum_cast<KindType>(var.value_string()) };
                if (kind.has_value())
                {
                    building.kind = kind.value();
                }
            }
            else if (var.name() == "Posoffs")
            {
                building.posoffs = var.value_int();
            }
            else if (var.name() == "Wegspeed")
            {
                for (auto v{ 0 }; v < var.value_array().value_size(); ++v)
                {
                    building.wegspeed.push_back(var.value_array().value(v).value_int());
                }
            }
            else if (var.name() == "Highflg")
            {
                building.highflg = var.value_int();
            }
            else if (var.name() == "Einhoffs")
            {
                building.einhoffs = var.value_int();
            }
            else if (var.name() == "Bausample")
            {
                const auto bausample{ magic_enum::enum_cast<BausampleType>(var.value_string()) };
                if (bausample.has_value())
                {
                    building.bausample = bausample.value();
                }
            }
            else if (var.name() == "Ruinenr")
            {
                const auto ruinenr{ magic_enum::enum_cast<RuinenrType>(var.value_string()) };
                if (ruinenr.has_value())
                {
                    building.ruinenr = ruinenr.value();
                }
            }
            else if (var.name() == "Maxenergy")
            {
                building.maxenergy = var.value_int();
            }
            else if (var.name() == "Maxbrand")
            {
                building.maxbrand = var.value_int();
            }
            else if (var.name() == "Size")
            {
                building.size.w = var.value_array().value(0).value_int();
                building.size.h = var.value_array().value(1).value_int();
            }
            else if (var.name() == "Rotate")
            {
                building.rotate = var.value_int();
            }
            else if (var.name() == "RandAnz")
            {
                building.randAnz = var.value_int();
            }
            else if (var.name() == "AnimTime")
            {
                if (var.value_string() == "TIMENEVER")
                {
                    building.animTime = -1;
                }
                else
                {
                    building.animTime = var.value_int();
                }
            }
            else if (var.name() == "AnimFrame")
            {
                building.animFrame = var.value_int();
            }
            else if (var.name() == "AnimAdd")
            {
                building.animAdd = var.value_int();
            }
            else if (var.name() == "Baugfx")
            {
                building.baugfx = var.value_int();
            }
            else if (var.name() == "PlaceFlg")
            {
                building.placeFlg = var.value_int();
            }
            else if (var.name() == "AnimAnz")
            {
                building.animAnz = var.value_int();
            }
            else if (var.name() == "KreuzBase")
            {
                building.kreuzBase = var.value_int();
            }
            else if (var.name() == "NoShotFlg")
            {
                building.noShotFlg = var.value_int();
            }
            else if (var.name() == "Strandflg")
            {
                building.strandflg = var.value_int();
            }
            else if (var.name() == "Ausbauflg")
            {
                building.ausbauflg = var.value_int();
            }
            else if (var.name() == "Tuerflg")
            {
                building.tuerflg = var.value_int();
            }
            else if (var.name() == "Randwachs")
            {
                building.randwachs = var.value_int();
            }
            else if (var.name() == "RandAdd")
            {
                building.randAdd = var.value_int();
            }
            else if (var.name() == "Strandoff")
            {
                building.strandoff = var.value_int();
            }
            else if (var.name() == "Destroyflg")
            {
                building.destroyflg = var.value_int();
            }
        }
    }

    if (t_obj->objects_size() > 0)
    {
        for (auto i{ 0 }; i < t_obj->objects_size(); ++i)
        {
            const auto& nestedObj{ t_obj->objects(i) };
            if (nestedObj.name() == "HAUS_PRODTYP")
            {
                for (auto j{ 0 }; j < nestedObj.variables().variable_size(); ++j)
                {
                    const auto var{ nestedObj.variables().variable(j) };
                    if (var.name() == "Kind")
                    {
                        const auto kind{ magic_enum::enum_cast<ProdtypKindType>(var.value_string()) };
                        if (kind.has_value())
                        {
                            building.houseProductionType.kind = kind.value();
                        }
                    }
                    else if (var.name() == "Ware")
                    {
                        const auto ware{ magic_enum::enum_cast<WareType>(var.value_string()) };
                        if (ware.has_value())
                        {
                            building.houseProductionType.ware = ware.value();
                        }
                    }
                    else if (var.name() == "Workstoff")
                    {
                        const auto workstoff{ magic_enum::enum_cast<WorkstoffType>(var.value_string()) };
                        if (workstoff.has_value())
                        {
                            building.houseProductionType.workstoff = workstoff.value();
                        }
                    }
                    else if (var.name() == "Erzbergnr")
                    {
                        const auto erzbergnr{ magic_enum::enum_cast<ErzbergnrType>(var.value_string()) };
                        if (erzbergnr.has_value())
                        {
                            building.houseProductionType.erzbergnr = erzbergnr.value();
                        }
                    }
                    else if (var.name() == "Rohstoff")
                    {
                        const auto rohstoff{ magic_enum::enum_cast<RohstoffType>(var.value_string()) };
                        if (rohstoff.has_value())
                        {
                            building.houseProductionType.rohstoff = rohstoff.value();
                        }
                    }
                    else if (var.name() == "MAXPRODCNT")
                    {
                        const auto maxprodcnt{ magic_enum::enum_cast<MaxprodcntType>(var.value_string()) };
                        if (maxprodcnt.has_value())
                        {
                            building.houseProductionType.maxprodcnt = maxprodcnt.value();
                        }
                    }
                    else if (var.name() == "Bauinfra")
                    {
                        const auto bauinfra{ magic_enum::enum_cast<BauinfraType>(var.value_string()) };
                        if (bauinfra.has_value())
                        {
                            building.houseProductionType.bauinfra = bauinfra.value();
                        }
                    }
                    else if (var.name() == "Figurnr")
                    {
                        const auto figurnr{ magic_enum::enum_cast<FigurnrType>(var.value_string()) };
                        if (figurnr.has_value())
                        {
                            building.houseProductionType.figurnr = figurnr.value();
                        }
                    }
                    else if (var.name() == "Rauchfignr")
                    {
                        const auto rauchfignr{ magic_enum::enum_cast<RauchfignrType>(var.value_string()) };
                        if (rauchfignr.has_value())
                        {
                            building.houseProductionType.rauchfignr = rauchfignr.value();
                        }
                    }
                    else if (var.name() == "Maxware")
                    {
                        for (auto v{ 0 }; v < var.value_array().value_size(); ++v)
                        {
                            building.houseProductionType.maxware.push_back(var.value_array().value(v).value_int());
                        }
                    }
                    else if (var.name() == "Kosten")
                    {
                        for (auto v{ 0 }; v < var.value_array().value_size(); ++v)
                        {
                            building.houseProductionType.kosten.push_back(var.value_array().value(v).value_int());
                        }
                    }
                    else if (var.name() == "BGruppe")
                    {
                        building.houseProductionType.bGruppe = var.value_int();
                    }
                    else if (var.name() == "LagAniFlg")
                    {
                        building.houseProductionType.lagAniFlg = var.value_int();
                    }
                    else if (var.name() == "NoMoreWork")
                    {
                        building.houseProductionType.noMoreWork = var.value_int();
                    }
                    else if (var.name() == "Workmenge")
                    {
                        building.houseProductionType.workmenge = var.value_int();
                    }
                    else if (var.name() == "Doerrflg")
                    {
                        building.houseProductionType.doerrflg = var.value_int();
                    }
                    else if (var.name() == "Anicontflg")
                    {
                        building.houseProductionType.anicontflg = var.value_int();
                    }
                    else if (var.name() == "MakLagFlg")
                    {
                        building.houseProductionType.makLagFlg = var.value_int();
                    }
                    else if (var.name() == "Nativflg")
                    {
                        building.houseProductionType.nativflg = var.value_int();
                    }
                    else if (var.name() == "NoLagVoll")
                    {
                        building.houseProductionType.noLagVoll = var.value_int();
                    }
                    else if (var.name() == "Radius")
                    {
                        building.houseProductionType.radius = var.value_int();
                    }
                    else if (var.name() == "Rohmenge")
                    {
                        building.houseProductionType.rohmenge = var.value_int();
                    }
                    else if (var.name() == "Prodmenge")
                    {
                        building.houseProductionType.prodmenge = var.value_int();
                    }
                    else if (var.name() == "Randwachs")
                    {
                        building.houseProductionType.randwachs = var.value_int();
                    }
                    else if (var.name() == "Maxlager")
                    {
                        building.houseProductionType.maxlager = var.value_int();
                    }
                    else if (var.name() == "Maxnorohst")
                    {
                        building.houseProductionType.maxnorohst = var.value_int();
                    }
                    else if (var.name() == "Arbeiter")
                    {
                        building.houseProductionType.arbeiter = var.value_int();
                    }
                    else if (var.name() == "Figuranz")
                    {
                        building.houseProductionType.figuranz = var.value_int();
                    }
                    else if (var.name() == "Interval")
                    {
                        building.houseProductionType.interval = var.value_int();
                    }
                }
            }
            else if (nestedObj.name() == "HAUS_BAUKOST")
            {
                for (auto j{ 0 }; j < nestedObj.variables().variable_size(); ++j)
                {
                    const auto var{ nestedObj.variables().variable(j) };
                    if (var.name() == "Money")
                    {
                        building.houseBuildCosts.money = var.value_int();
                    }
                    else if (var.name() == "Werkzeug")
                    {
                        building.houseBuildCosts.werkzeug = var.value_int();
                    }
                    else if (var.name() == "Holz")
                    {
                        building.houseBuildCosts.holz = var.value_int();
                    }
                    else if (var.name() == "Ziegel")
                    {
                        building.houseBuildCosts.ziegel = var.value_int();
                    }
                    else if (var.name() == "Kanon")
                    {
                        building.houseBuildCosts.kanon = var.value_int();
                    }
                }
            }
        }
    }

    return building;
}
